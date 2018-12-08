/*
 * smart_switch.h
 Copyright 2018 Jeffrey Friesen  jeff@enertionboards.com

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



#ifndef SMART_SWITCH_H_
#define SMART_SWITCH_H_

#include "mc_interface.h"

void smart_switch_pin_init(void);
void smart_switch_set_conf(smart_switch_config *conf);
void smart_switch_keep_on(void);
void smart_switch_shut_down(void);
bool smart_switch_is_pressed(void);
void smart_switch_thread_start(void);


#endif /* SMART_SWITCH_H_ */
