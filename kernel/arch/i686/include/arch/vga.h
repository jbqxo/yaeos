#pragma once

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_ADDR 0x000B8000u

typedef uint8_t vga_char_color;

enum vga_color {
	VGA_BLACK = 0,
	VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LGRAY,
	VGA_DGRAY,
	VGA_LBLUE,
	VGA_LGREEN,
	VGA_LCYAN,
	VGA_LRED,
	VGA_LMAGENTA,
	VGA_YELLOW,
	VGA_WHITE
};

struct vga_state {
	uint16_t *buffer;
	uint_fast16_t row;
	uint_fast16_t col;
	uint8_t color;
};

#define VGA_DEFAULT_FG VGA_WHITE
#define VGA_DEFAULT_BG VGA_BLACK

static inline vga_char_color vga_mix_color(enum vga_color fg, enum vga_color bg)
{
	return (vga_char_color)(fg | bg << 4);
}

static inline uint16_t vga_char(unsigned char uc, vga_char_color color)
{
	return (uint16_t)uc | (uint16_t)(color << 8);
}
