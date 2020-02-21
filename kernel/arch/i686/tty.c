#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

#include <arch/vga.h>
#include <arch/platform.h>

static struct {
	uint16_t *buffer;
	uint_fast16_t row;
	uint_fast16_t col;
	uint8_t color;
} state;

static void write_char(uint8_t c, vga_char_color color, uint_fast16_t x,
		       uint_fast16_t y)
{
	const uint_fast16_t index = y * VGA_WIDTH + x;
	state.buffer[index] = vga_char(c, color);
}

void tty_scroll_down(void)
{
	for (uint_fast16_t y = 0; y < VGA_HEIGHT - 1; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			uint_fast16_t dst_i = y * VGA_WIDTH + x;
			uint_fast16_t src_i = dst_i + VGA_WIDTH;
			state.buffer[dst_i] = state.buffer[src_i];
		}
	}
}

void tty_init(void)
{
	state.row = 0;
	state.col = 0;
	state.color = vga_mix_color(vga_fg_color, vga_bg_color);
	state.buffer = (uint16_t *)PLATFORM_VGA_BUFFER;

	for (uint_fast16_t y = 0; y < VGA_HEIGHT; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			write_char(' ', state.color, x, y);
		}
	}
}

void tty_putchar(char c)
{
	if (c == '\n' || state.col == VGA_WIDTH) {
		state.col = 0;
		if (++state.row == VGA_HEIGHT) {
			tty_scroll_down();
			state.row--;
			for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
				write_char(' ', state.color, x, state.row);
			}
		}
	}

	if (isprint(c)) {
		write_char(c, state.color, state.col, state.row);
		state.col++;
	}
}

void tty_write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		tty_putchar(data[i]);
	}
}

void tty_writestring(const char *str)
{
	while (*str) {
		tty_putchar(*str);
		str++;
	}
}
