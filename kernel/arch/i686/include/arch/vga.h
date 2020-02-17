#pragma once

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFF_ADDR 0xB8000

#include <stdint.h>

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

const enum vga_color vga_fg_color = VGA_WHITE;
const enum vga_color vga_bg_color = VGA_BLACK;

static inline vga_char_color vga_mix_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_char(unsigned char uc, vga_char_color color)
{
	return (uint16_t)uc | (uint16_t)color << 8;
}
