/*
 */

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>

#include "max72xx.h"

#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

struct Max72xxConfig ICACHE_FLASH_ATTR max72xx_configure(int clkPin, int csPin,
		int dataPin, int numDevices) {
	struct Max72xxConfig cfg;

	cfg.pinDATA = dataPin;
	cfg.pinCLK = clkPin;
	cfg.pinCS = csPin;

	if (numDevices <= 0 || numDevices > 8)
		numDevices = 8;

	cfg.maxDevices = numDevices;

	uint8 i;
	for (i = 0; i < 64; i++)
		cfg.buf[i] = 0x00;

	return cfg;
}

void ICACHE_FLASH_ATTR max72xx_start(struct Max72xxConfig cfg) {
	GPIO_OUTPUT_SET(cfg.pinCS, 0);
}

void ICACHE_FLASH_ATTR max72xx_complete(struct Max72xxConfig cfg) {
	GPIO_OUTPUT_SET(cfg.pinCS, 1);
}

void ICACHE_FLASH_ATTR max72xx_cmd(struct Max72xxConfig cfg, volatile uint8 cmd,
		volatile uint8 val) {
	uint8 i;
	for (i = 0; i < 8; i++) {
		GPIO_OUTPUT_SET(cfg.pinDATA, !!(cmd & (1 << (7 - i))));
		GPIO_OUTPUT_SET(cfg.pinCLK, 0x01);
		GPIO_OUTPUT_SET(cfg.pinCLK, 0x00);
	}

	for (i = 0; i < 8; i++) {
		GPIO_OUTPUT_SET(cfg.pinDATA, !!(val & (1 << (7 - i))));
		GPIO_OUTPUT_SET(cfg.pinCLK, 0x01);
		GPIO_OUTPUT_SET(cfg.pinCLK, 0x00);
	}
}

void ICACHE_FLASH_ATTR max72xx_setCommand(struct Max72xxConfig cfg,
		uint8 command, uint8 value) {
	uint8 i;

	max72xx_start(cfg);
	for (i = 0; i < cfg.maxDevices; i++) {
		max72xx_cmd(cfg, command, value);
	}
	max72xx_complete(cfg);
}

void ICACHE_FLASH_ATTR max72xx_init(struct Max72xxConfig cfg) {
	GPIO_OUTPUT_SET(cfg.pinCS, 1);

	max72xx_setCommand(cfg, MAX72XX_SCANLIMIT, 0x07);
	max72xx_setCommand(cfg, MAX72XX_DECMODE, 0x00); // using an led matrix (not digits)
	max72xx_setCommand(cfg, MAX72XX_DISPTEST, 0x00); // no display test

	max72xx_clear(cfg); // empty registers, turn all LEDs off
	max72xx_setCommand(cfg, MAX72XX_SHUTDOWN, 0x01); //we go into shutdown-mode on startup

	max72xx_setIntensity(cfg, 0x0f);  // the first 0x0f is the value you can set
}

void ICACHE_FLASH_ATTR max72xx_setIntensity(struct Max72xxConfig cfg,
		uint8 intensity) {
	if (intensity >= 0 && intensity < 16)
		max72xx_setCommand(cfg, MAX72XX_INTENSITY, intensity);
}

void ICACHE_FLASH_ATTR max72xx_setDot(struct Max72xxConfig cfg, uint8 col,
		uint8 row, uint8 value) {
	uint8 i;
	uint8 n = col / 8;
	uint8 c = col % 8;

	bitWrite(cfg.buf[col], row, value);

	max72xx_start(cfg);
	for (i = 0; i < n; i++) {
		if (i == n) {
			max72xx_cmd(cfg, c + 1, cfg.buf[col]);
		} else {
			max72xx_cmd(cfg, 0, 0);
		}
	}
	max72xx_complete(cfg);
}

void ICACHE_FLASH_ATTR max72xx_showChar(struct Max72xxConfig cfg, int matrix,
		int charIndex) {
	uint8 i;
	uint8 ofs = matrix * 8;

	for (i = 0; i < 6; i++) {
		max72xx_setColumn(cfg, ofs + i, max72xxFont[charIndex][i]);
	}
}

void ICACHE_FLASH_ATTR max72xx_setColumn(struct Max72xxConfig cfg, uint8 col,
		uint8 value) {
	int n = col / 8;
	int c = col % 8;

	max72xx_start(cfg);

	uint8 i;
	for (i = 0; i < cfg.maxDevices; i++) {
		if (i == n) {
			max72xx_cmd(cfg, c + 1, value);
		} else {
			max72xx_cmd(cfg, 0, 0);
		}
	}

	cfg.buf[col] = value;

	max72xx_complete(cfg);
}

void ICACHE_FLASH_ATTR max72xx_setColumnAll(struct Max72xxConfig cfg, uint8 col,
		uint8 value) {
	uint8 i;

	max72xx_start(cfg);

	for (i = 0; i < cfg.maxDevices; i++) {
		max72xx_cmd(cfg, col + 1, value);

		cfg.buf[col * i] = value;
	}

	max72xx_complete(cfg);
}

void ICACHE_FLASH_ATTR max72xx_clear(struct Max72xxConfig cfg) {
	uint8 i;

	for (i = 0; i < 8; i++)
		max72xx_setColumnAll(cfg, i, 0);
}

void ICACHE_FLASH_ATTR max72xx_refresh(struct Max72xxConfig cfg) {
	uint8 i;
	for (i = 0; i < 8; i++) {
		int col = i;
		uint8 j;

		max72xx_start(cfg);
		for (j = 0; j < cfg.maxDevices; j++) {
			max72xx_cmd(cfg, i + 1, cfg.buf[col]);
			col += 8;
		}
		max72xx_complete(cfg);
	}
}
