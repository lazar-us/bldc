/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef MCPWM2_H_
#define MCPWM2_H_

#include "conf_general.h"

// Functions
void mcpwm_init2(volatile mc_configuration *configuration);
void mcpwm_deinit2(void);
bool mcpwm_init_done2(void);
void mcpwm_set_configuration2(volatile mc_configuration *configuration);
void mcpwm_init_hall_table2(int8_t *table);
void mcpwm_set_duty2(float dutyCycle);
void mcpwm_set_duty_noramp2(float dutyCycle);
void mcpwm_set_pid_speed2(float rpm);
void mcpwm_set_pid_pos2(float pos);
void mcpwm_set_current2(float current);
void mcpwm_set_current_dccal2(int off0, int off1);
void mcpwm_set_brake_current2(float current);
void mcpwm_brake_now2(void);
void mcpwm_release_motor2(void);
int mcpwm_get_comm_step2(void);
float mcpwm_get_duty_cycle_set2(void);
float mcpwm_get_duty_cycle_now2(void);
float mcpwm_get_switching_frequency_now2(void);
float mcpwm_get_rpm2(void);
mc_state mcpwm_get_state2(void);
float mcpwm_get_kv2(void);
float mcpwm_get_kv_filtered2(void);
int mcpwm_get_tachometer_value2(bool reset);
int mcpwm_get_tachometer_abs_value2(bool reset);
void mcpwm_stop_pwm2(void);
unsigned int mcpwm_get_timer_duty2(void);
float mcpwm_get_tot_current2(void);
float mcpwm_get_tot_current_filtered2(void);
float mcpwm_get_tot_current_directional2(void);
float mcpwm_get_tot_current_directional_filtered2(void);
float mcpwm_get_tot_current_in2(void);
float mcpwm_get_tot_current_in_filtered2(void);
void mcpwm_set_detect2(void);
float mcpwm_get_detect_pos2(void);
void mcpwm_reset_hall_detect_table2(void);
int mcpwm_get_hall_detect_result2(int8_t *table);
int mcpwm_read_hall_phase2(void);
float mcpwm_read_reset_avg_cycle_integrator2(void);
void mcpwm_set_comm_mode2(mc_comm_mode mode);
mc_comm_mode mcpwm_get_comm_mode2(void);
float mcpwm_get_last_adc_isr_duration2(void);
float mcpwm_get_last_inj_adc_isr_duration2(void);
mc_rpm_dep_struct mcpwm_get_rpm_dep2(void);
bool mcpwm_is_dccal_done2(void);
void mcpwm_switch_comm_mode2(mc_comm_mode next);

// Interrupt handlers
void mcpwm_adc_inj_int_handler2(int curr0, int curr1);
void mcpwm_adc_int_handler2(void);

// External variables
extern volatile float mcpwm_detect_currents2[];
extern volatile float mcpwm_detect_voltages2[];
extern volatile float mcpwm_detect_currents_diff2[];
extern volatile int mcpwm_vzero2;

/*
 * Fixed parameters
 */
#define MCPWM_RPM_TIMER_FREQ			1000000.0	// Frequency of the RPM measurement timer
#define MCPWM_CMD_STOP_TIME				0		// Ignore commands for this duration in msec after a stop has been sent
#define MCPWM_DETECT_STOP_TIME			500		// Ignore commands for this duration in msec after a detect command

// Speed PID parameters
#define MCPWM_PID_TIME_K				0.001	// Pid controller sample time in seconds

#endif /* MC_PWM2_H_ */
