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
#ifndef ENCODER2_H_
#define ENCODER2_H_

#include "conf_general.h"

// Functions
void encoder_deinit2(void);
void encoder_init_abi2(uint32_t counts);
void encoder_init_as5047p_spi2(void);
bool encoder_is_configured2(void);
float encoder_read_deg2(void);
void encoder_reset2(void);
void encoder_tim_isr2(void);
void encoder_set_counts2(uint32_t counts);
bool encoder_index_found2(void);

#endif /* ENCODER_H_ */
