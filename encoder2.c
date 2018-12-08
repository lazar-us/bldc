/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// TODO make encoder 2 work.

#include "encoder2.h"
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"
#include "hw.h"
#include "utils.h"

// Defines
#define AS5047P_READ_ANGLECOM		(0x3FFF | 0x4000 | 0x8000) // This is just ones
#define AS5047_SAMPLE_RATE_HZ		20000


#define SPI_SW_MISO_GPIO2			HW_HALL_ENC_GPIO5
#define SPI_SW_MISO_PIN2		    HW_HALL_ENC_PIN5
#define SPI_SW_SCK_GPIO2		    HW_HALL_ENC_GPIO4
#define SPI_SW_SCK_PIN2				HW_HALL_ENC_PIN4
#define SPI_SW_CS_GPIO2				HW_HALL_ENC_GPIO6
#define SPI_SW_CS_PIN2				HW_HALL_ENC_PIN6


// Private types
typedef enum {
	ENCODER_MODE_NONE = 0,
	ENCODER_MODE_ABI,
	ENCODER_MODE_AS5047P_SPI
} encoder_mode;

// Private variables
static bool index_found = false;
static uint32_t enc_counts = 10000;
static encoder_mode mode = ENCODER_MODE_NONE;
static float last_enc_angle = 0.0;

// Private functions
static void spi_transfer(uint16_t *in_buf, const uint16_t *out_buf, int length);
static void spi_begin(void);
static void spi_end(void);
static void spi_delay(void);

void encoder_deinit2(void) {
	nvicDisableVector(HW_ENC_EXTI_CH2);
	nvicDisableVector(HW_ENC_TIM_ISR_CH2);

	TIM_DeInit(HW_ENC_TIM2);

	palSetPadMode(SPI_SW_MISO_GPIO2, SPI_SW_MISO_PIN2, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(SPI_SW_SCK_GPIO2, SPI_SW_SCK_PIN2, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(SPI_SW_CS_GPIO2, SPI_SW_CS_PIN2, PAL_MODE_INPUT_PULLUP);

	palSetPadMode(HW_HALL_ENC_GPIO4, HW_HALL_ENC_PIN4, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO5, HW_HALL_ENC_PIN5, PAL_MODE_INPUT_PULLUP);

	index_found = false;
	mode = ENCODER_MODE_NONE;
	last_enc_angle = 0.0;
}

void encoder_init_abi2(uint32_t counts) {
	EXTI_InitTypeDef   EXTI_InitStructure;

	// Initialize variables
	index_found = false;
	enc_counts = counts;

	palSetPadMode(HW_HALL_ENC_GPIO4, HW_HALL_ENC_PIN4, PAL_MODE_ALTERNATE(HW_ENC_TIM_AF));
	palSetPadMode(HW_HALL_ENC_GPIO5, HW_HALL_ENC_PIN5, PAL_MODE_ALTERNATE(HW_ENC_TIM_AF));
//	palSetPadMode(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3, PAL_MODE_ALTERNATE(HW_ENC_TIM_AF));

	// Enable timer clock
	HW_ENC_TIM_CLK_EN2();

	// Enable SYSCFG clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	TIM_EncoderInterfaceConfig (HW_ENC_TIM2, TIM_EncoderMode_TI12,
			TIM_ICPolarity_Rising,
			TIM_ICPolarity_Rising);
	TIM_SetAutoreload(HW_ENC_TIM2, enc_counts - 1);

	// Filter
	HW_ENC_TIM2->CCMR1 |= 6 << 12 | 6 << 4;
	HW_ENC_TIM2->CCMR2 |= 6 << 4;

	TIM_Cmd(HW_ENC_TIM2, ENABLE);

	// Interrupt on index pulse

	// Connect EXTI Line to pin
	SYSCFG_EXTILineConfig(HW_ENC_EXTI_PORTSRC2, HW_ENC_EXTI_PINSRC2);

	// Configure EXTI Line
	EXTI_InitStructure.EXTI_Line = HW_ENC_EXTI_LINE2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set EXTI Line Interrupt to the highest priority
	nvicEnableVector(HW_ENC_EXTI_CH2, 0);

	mode = ENCODER_MODE_ABI;
}

void encoder_init_as5047p_spi2(void) {
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	palSetPadMode(SPI_SW_MISO_GPIO2, SPI_SW_MISO_PIN2, PAL_MODE_INPUT);
	palSetPadMode(SPI_SW_SCK_GPIO2, SPI_SW_SCK_PIN2, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SPI_SW_CS_GPIO2, SPI_SW_CS_PIN2, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);


	// Enable timer clock
	HW_ENC_TIM_CLK_EN2();

	// Time Base configuration
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = ((168000000 / 2 / AS5047_SAMPLE_RATE_HZ) - 1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(HW_ENC_TIM2, &TIM_TimeBaseStructure);

	// Enable overflow interrupt
	TIM_ITConfig(HW_ENC_TIM2, TIM_IT_Update, ENABLE);

	// Enable timer
	TIM_Cmd(HW_ENC_TIM2, ENABLE);

	nvicEnableVector(HW_ENC_TIM_ISR_CH2, 6);

	mode = ENCODER_MODE_AS5047P_SPI;
	index_found = true;
}

bool encoder_is_configured2(void) {
	return mode != ENCODER_MODE_NONE;
}

float encoder_read_deg2(void) {
	static float angle = 0.0;

	switch (mode) {
	case ENCODER_MODE_ABI:
		angle = ((float)HW_ENC_TIM2->CNT * 360.0) / (float)enc_counts;
		break;

	case ENCODER_MODE_AS5047P_SPI:
		angle = last_enc_angle;
		break;

	default:
		break;
	}

	return angle;
}

/**
 * Reset the encoder counter. Should be called from the index interrupt.
 */
void encoder_reset2(void) {
	// Only reset if the pin is still high to avoid too short pulses, which
	// most likely are noise.
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	if (palReadPad(HW_HALL_ENC_GPIO6, HW_HALL_ENC_PIN6)) {
		const unsigned int cnt = HW_ENC_TIM2->CNT;
		static int bad_pulses = 0;
		const unsigned int lim = enc_counts / 20;

		if (index_found) {
			// Some plausibility filtering.
			if (cnt > (enc_counts - lim) || cnt < lim) {
				HW_ENC_TIM2->CNT = 0;
				bad_pulses = 0;
			} else {
				bad_pulses++;

				if (bad_pulses > 5) {
					index_found = 0;
				}
			}
		} else {
			HW_ENC_TIM2->CNT = 0;
			index_found = true;
			bad_pulses = 0;
		}
	}
}

/**
 * Timer interrupt
 */
void encoder_tim_isr2(void) {
	uint16_t pos;

	spi_begin();
	spi_transfer(&pos, 0, 1);
	spi_end();

	pos &= 0x3FFF;
	last_enc_angle = ((float)pos * 360.0) / 16384.0;
}

/**
 * Set the number of encoder counts.
 *
 * @param counts
 * The number of encoder counts
 */
void encoder_set_counts2(uint32_t counts) {
	if (counts != enc_counts) {
		enc_counts = counts;
		TIM_SetAutoreload(HW_ENC_TIM2, enc_counts - 1);
		index_found = false;
	}
}

/**
 * Check if the index pulse is found.
 *
 * @return
 * True if the index is found, false otherwise.
 */
bool encoder_index_found2(void) {
	return index_found;
}

// Software SPI
static void spi_transfer(uint16_t *in_buf, const uint16_t *out_buf, int length) {
	for (int i = 0;i < length;i++) {
		uint16_t send = out_buf ? out_buf[i] : 0xFFFF;
		uint16_t recieve = 0;

		for (int bit = 0;bit < 16;bit++) {
			//palWritePad(HW_SPI_PORT_MOSI, HW_SPI_PIN_MOSI, send >> 15);
			send <<= 1;

			spi_delay();
			palSetPad(SPI_SW_SCK_GPIO2, SPI_SW_SCK_PIN2);
			spi_delay();

			int r1, r2, r3;
			r1 = palReadPad(SPI_SW_MISO_GPIO2, SPI_SW_MISO_PIN2);
			__NOP();
			r2 = palReadPad(SPI_SW_MISO_GPIO2, SPI_SW_MISO_PIN2);
			__NOP();
			r3 = palReadPad(SPI_SW_MISO_GPIO2, SPI_SW_MISO_PIN2);

			recieve <<= 1;
			if (utils_middle_of_3_int(r1, r2, r3)) {
				recieve |= 1;
			}

			palClearPad(SPI_SW_SCK_GPIO2, SPI_SW_SCK_PIN2);
			spi_delay();
		}

		if (in_buf) {
			in_buf[i] = recieve;
		}
	}
}

static void spi_begin(void) {
	palClearPad(SPI_SW_CS_GPIO2, SPI_SW_CS_PIN2);
}

static void spi_end(void) {
	palSetPad(SPI_SW_CS_GPIO2, SPI_SW_CS_PIN2);
}

static void spi_delay(void) {
	__NOP();
	__NOP();
	__NOP();
	__NOP();
}
