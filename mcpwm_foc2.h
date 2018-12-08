/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se
    Copyright 2018 Jeffrey Friesen jfriesen222@gmail.com

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
#ifndef MCPWM_FOC2_H_
#define MCPWM_FOC2_H_

#include "conf_general.h"
#include "datatypes.h"
#include <stdbool.h>

// Functions
void mcpwm_foc_init2(volatile mc_configuration *configuration);
void mcpwm_foc_deinit2(void);
bool mcpwm_foc_init_done2(void);
void mcpwm_foc_set_configuration2(volatile mc_configuration *configuration);
mc_state mcpwm_foc_get_state2(void);
void mcpwm_foc_start_resistance2(float current);
void mcpwm_foc_stop_resistance2(void);
bool mcpwm_foc_is_dccal_done2(void);
void mcpwm_foc_stop_pwm2(void);
void mcpwm_foc_set_duty2(float dutyCycle);
void mcpwm_foc_set_duty_noramp2(float dutyCycle);
void mcpwm_foc_set_pid_speed2(float rpm);
void mcpwm_foc_set_pid_pos2(float pos);
void mcpwm_foc_set_current2(float current);
void mcpwm_foc_set_brake_current2(float current);
void mcpwm_foc_set_handbrake2(float current);
void mcpwm_foc_set_openloop2(float current, float rpm);
float mcpwm_foc_get_duty_cycle_set2(void);
float mcpwm_foc_get_duty_cycle_now2(void);
float mcpwm_foc_get_pid_pos_set2(void);
float mcpwm_foc_get_pid_pos_now2(void);
float mcpwm_foc_get_switching_frequency_now2(void);
float mcpwm_foc_get_sampling_frequency_now2(void);
float mcpwm_foc_get_rpm2(void);
float mcpwm_foc_get_tot_current2(void);
float mcpwm_foc_get_tot_current_filtered2(void);
float mcpwm_foc_get_abs_motor_current2(void);
float mcpwm_foc_get_abs_motor_voltage2(void);
float mcpwm_foc_get_abs_motor_current_filtered2(void);
float mcpwm_foc_get_tot_current_directional2(void);
float mcpwm_foc_get_tot_current_directional_filtered2(void);
float mcpwm_foc_get_id2(void);
float mcpwm_foc_get_iq2(void);
float mcpwm_foc_get_tot_current_in2(void);
float mcpwm_foc_get_tot_current_in_filtered2(void);
int mcpwm_foc_get_tachometer_value2(bool reset);
int mcpwm_foc_get_tachometer_abs_value2(bool reset);
float mcpwm_foc_get_phase2(void);
float mcpwm_foc_get_phase_observer2(void);
float mcpwm_foc_get_phase_encoder2(void);
float mcpwm_foc_get_vd2(void);
float mcpwm_foc_get_vq2(void);
void mcpwm_foc_encoder_detect2(float current, bool print, float *offset, float *ratio, bool *inverted);
float mcpwm_foc_measure_resistance2(float current, int samples);
float mcpwm_foc_measure_inductance2(float duty, int samples, float *curr);
bool mcpwm_foc_measure_res_ind2(float *res, float *ind);
bool mcpwm_foc_hall_detect2(float current, uint8_t *hall_table);
void mcpwm_foc_print_state2(void);
float mcpwm_foc_get_last_inj_adc_isr_duration2(void);
void mcpwm_foc_hall_detect_start2(float current);
void mcpwm_foc_set_phase_override2(float phase_now);
void mcpwm_foc_hall_detect_end2(void);

// Interrupt handlers
void mcpwm_foc_tim_sample_int_handler2(void);
//motor handler
void mcpwm_set_curr_offset(int off0, int off1) ;
void mcpwm_foc_handler2(void);


// Defines
#define MCPWM_FOC_INDUCTANCE_SAMPLE_CNT_OFFSET		10 // Offset for the inductance measurement sample time in timer ticks
#define MCPWM_FOC_INDUCTANCE_SAMPLE_RISE_COMP		50 // Current rise time compensation
#define MCPWM_FOC_I_FILTER_CONST					0.1 // Filter constant for the current filters

#endif /* MCPWM_FOC_H_ */
