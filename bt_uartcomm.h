/*
 * bt_uartcomm.h
 *
 *  Created on: Oct 7, 2018
 *      Author: jfree
 */

#ifndef BT_UARTCOMM_H_
#define BT_UARTCOMM_H_

#include "conf_general.h"

void bt_uartcomm_start(void);
void bt_uartcomm_stop(void);
void bt_uartcomm_configure(uint32_t baudrate);

#endif /* BT_UARTCOMM_H_ */
