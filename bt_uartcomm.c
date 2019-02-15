/*
	Copyright 2017 Benjamin Vedder	benjamin@vedder.se

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

#include "app.h"
#include "ch.h"
#include "hal.h"
#include "hw.h"
#include "packet.h"
#include "commands.h"

#include <string.h>

// Settings
#ifdef USE_ALT_UART_PORT
#define BAUDRATE                    115200
#else
#define BAUDRATE                    250000
#endif
#define PACKET_HANDLER              2
#define SERIAL_RX_BUFFER_SIZE       2048

// Threads
static THD_FUNCTION(packet_process_thread2, arg);
static THD_WORKING_AREA(packet_process_thread2_wa, 4096);
static thread_t *process_tp = 0;

// Variables
static uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_SIZE];
static int serial_rx_read_pos = 0;
static int serial_rx_write_pos = 0;
static volatile bool is_running = false;

// Private functions
static void process_packet(unsigned char *data, unsigned int len);
static void send_packet_wrapper(unsigned char *data, unsigned int len);
static void send_packet(unsigned char *data, unsigned int len);

/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {
    (void)uartp;
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void txend2(UARTDriver *uartp) {
    (void)uartp;
}

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void rxerr(UARTDriver *uartp, uartflags_t e) {
    (void)uartp;
    (void)e;
}

/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void rxchar(UARTDriver *uartp, uint16_t c) {
    (void)uartp;
    serial_rx_buffer[serial_rx_write_pos++] = c;

    if (serial_rx_write_pos == SERIAL_RX_BUFFER_SIZE) {
        serial_rx_write_pos = 0;
    }

    chSysLockFromISR();
    chEvtSignalI(process_tp, (eventmask_t) 1);
    chSysUnlockFromISR();
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void rxend(UARTDriver *uartp) {
    (void)uartp;
}

/*
 * UART driver configuration structure.
 */
static UARTConfig uart_cfg = {
        txend1,
        txend2,
        rxend,
        rxchar,
        rxerr,
        BAUDRATE,
        0,
        USART_CR2_LINEN,
        0
};

static void process_packet(unsigned char *data, unsigned int len) {
    commands_set_send_func(send_packet_wrapper);
    commands_process_packet(data, len);
}

static void send_packet_wrapper(unsigned char *data, unsigned int len) {
    packet_send_packet(data, len, PACKET_HANDLER);
}

static void send_packet(unsigned char *data, unsigned int len) {
    // Wait for the previous transmission to finish.
    while (BT_UART_DEV.txstate == UART_TX_ACTIVE) {
        chThdSleep(1);
    }

    // Copy this data to a new buffer in case the provided one is re-used
    // after this function returns.
    static uint8_t buffer[PACKET_MAX_PL_LEN + 5];
    memcpy(buffer, data, len);

    uartStartSend(&BT_UART_DEV, len, buffer);
}

void bt_uartcomm_start(void) {
    packet_init(send_packet, process_packet, PACKET_HANDLER);
    serial_rx_read_pos = 0;
    serial_rx_write_pos = 0;

    if (!is_running) {
        chThdCreateStatic(packet_process_thread2_wa, sizeof(packet_process_thread2_wa),
                HIGHPRIO, packet_process_thread2, NULL);
        is_running = true;
    }

    uartStart(&BT_UART_DEV, &uart_cfg);
    palSetPadMode(BT_UART_TX_PORT, BT_UART_TX_PIN, PAL_MODE_ALTERNATE(BT_UART_GPIO_AF) |
            PAL_STM32_OSPEED_HIGHEST |
            PAL_STM32_PUDR_FLOATING);
    palSetPadMode(BT_UART_RX_PORT, BT_UART_RX_PIN, PAL_MODE_ALTERNATE(BT_UART_GPIO_AF) |
            PAL_STM32_OSPEED_HIGHEST |
            PAL_STM32_PUDR_FLOATING);
}

void bt_uartcomm_stop(void) {
    uartStop(&BT_UART_DEV);
    palSetPadMode(BT_UART_TX_PORT, BT_UART_TX_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(BT_UART_RX_PORT, BT_UART_RX_PIN, PAL_MODE_INPUT_PULLUP);

    // Notice that the processing thread is kept running in case this call is made from it.
}

void bt_uartcomm_configure(uint32_t baudrate) {
    uart_cfg.speed = baudrate;

    if (is_running) {
        uartStart(&BT_UART_DEV, &uart_cfg);
    }
}

static THD_FUNCTION(packet_process_thread2, arg) {
    (void)arg;

    chRegSetThreadName("BT uartcomm process");

    process_tp = chThdGetSelfX();

    for(;;) {
        chEvtWaitAny((eventmask_t) 1);

        while (serial_rx_read_pos != serial_rx_write_pos) {
            packet_process_byte(serial_rx_buffer[serial_rx_read_pos++], PACKET_HANDLER);

            if (serial_rx_read_pos == SERIAL_RX_BUFFER_SIZE) {
                serial_rx_read_pos = 0;
            }
        }
    }
}




