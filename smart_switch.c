/*
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

#include "ch.h"
#include "hal.h"
#include "hw.h"
#include "ledpwm.h"
#include "mc_interface.h"
static unsigned int millis_switch_pressed = 0;
static volatile smart_switch_config config;
static THD_WORKING_AREA(smart_switch_thread_wa, 128);

typedef enum {
  UNITY_BOOTED = 0,
  TURN_ON_DELAY_ACTIVE,
  SWITCH_HELD_AFTER_TURN_ON,
  TURNED_ON,
  SHUTTING_DOWN,
} switch_states;

static volatile switch_states switch_state = UNITY_BOOTED;

void smart_switch_pin_init(void) {
  palSetPadMode(SWITCH_IN_GPIO, SWITCH_IN_PIN, PAL_MODE_INPUT);
  palSetPadMode(SWITCH_OUT_GPIO,SWITCH_OUT_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(SWITCH_LED_1_GPIO,SWITCH_LED_1_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palSetPadMode(SWITCH_LED_2_GPIO,SWITCH_LED_2_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  palClearPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
  return;
}

void smart_switch_set_conf(smart_switch_config *conf){
  config = *conf;
}

void smart_switch_keep_on(void) {
  palSetPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
  palClearPad(SWITCH_LED_1_GPIO,SWITCH_LED_1_PIN);
  return;
}

void smart_switch_shut_down(void) {
  palClearPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
  return;
}

bool smart_switch_is_pressed(void) {

  if(palReadPad(SWITCH_IN_GPIO, SWITCH_IN_PIN) == 1)
    return true;
  else
    return false;
}

static THD_FUNCTION(smart_switch_thread, arg) {
  (void)arg;
  chRegSetThreadName("smart_switch_timer");
  for (;;) {

    switch (switch_state) {
    case UNITY_BOOTED:
      ledpwm_set_intensity(SWITCH_LED_1,0.4);
      if(config.push_to_start_enabled){
        switch_state = TURN_ON_DELAY_ACTIVE;
      }else if(smart_switch_is_pressed()){
        switch_state = TURN_ON_DELAY_ACTIVE;
      }else{
        switch_state = SHUTTING_DOWN;
      }
      break;
    case TURN_ON_DELAY_ACTIVE:
      switch_state = SWITCH_HELD_AFTER_TURN_ON;
      chThdSleepMilliseconds(500);
      break;
    case SWITCH_HELD_AFTER_TURN_ON:
      smart_switch_keep_on();
      if(smart_switch_is_pressed()){
        switch_state = SWITCH_HELD_AFTER_TURN_ON;
      }
      else{
        switch_state = TURNED_ON;
      }
      break;
    case TURNED_ON:
      if(smart_switch_is_pressed()){
        millis_switch_pressed++;
        ledpwm_set_intensity(SWITCH_LED_1,1.0);
      }else{
        millis_switch_pressed = 0;
        ledpwm_set_intensity(SWITCH_LED_1,0.4);
      }
      if(((millis_switch_pressed > config.msec_pressed_for_off) || (mc_interface_seconds_inactive() > config.sec_inactive_for_off)) && (config.sec_inactive_for_off > 0)){
        switch_state = SHUTTING_DOWN;
      }
      break;
    case SHUTTING_DOWN:
      ledpwm_set_intensity(SWITCH_LED_1,0);
      smart_switch_shut_down();
      break;
    default:
      break;
    }
    chThdSleepMilliseconds(1);
  }
}

void smart_switch_thread_start(void){
  chThdCreateStatic(smart_switch_thread_wa, sizeof(smart_switch_thread_wa), NORMALPRIO, smart_switch_thread, NULL);
}

