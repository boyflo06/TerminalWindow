#pragma once

#include <stdint.h>

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;

	color();
	color(uint8_t r, uint8_t g, uint8_t b);
};