/*
 The obligatory blinky demo
 Blink an LED on GPIO pin 2
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

#include "max72xx.h"

#define DELAY 200 /* milliseconds */

LOCAL struct Max72xxConfig maxCfg;
LOCAL os_timer_t matrix_timer;
LOCAL uint8 i = 0;

void setChar(uint8 matrix, uint8 code) {
	uint8 i, ofs = matrix * 8;

	for (i = 0; i < 6; i++) {
		maxCfg.buf[ofs + i + 1] = max72xxFont[code][i];
	}
}

LOCAL void ICACHE_FLASH_ATTR matrix_cb(void *arg) {
	//maxCfg.buf[i]++;

	i++;
	if (i > 37) {
		i = 15;
	}

	setChar(0, i);
	setChar(1, i + 1);
	setChar(2, i + 2);
	setChar(3, i + 3);

	max72xx_refresh(maxCfg);
}

void user_init(void) {
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

	maxCfg = max72xx_configure(2, 4, 5, 4);
	max72xx_init(maxCfg);
	max72xx_setIntensity(maxCfg, 5);

	// Set up a timer
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(&matrix_timer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(&matrix_timer, (os_timer_func_t *) matrix_cb, (void *) 0);
	// void os_timer_arm(ETSTimer *ptimer, uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(&matrix_timer, DELAY, 1);

}
