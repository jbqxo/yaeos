#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

#include <arch/vga.h>
#include <arch/platform.h>
#include <arch/vm.h>

#include <kernel/tty.h>

static struct vga_state GLOBAL_STATE;

static void write_char(tty_descriptor_t desc, uint8_t c, vga_char_color color,
		       uint_fast16_t x, uint_fast16_t y)
{
	struct vga_state *s = desc;
	const uint_fast16_t index = y * VGA_WIDTH + x;
	s->buffer[index] = vga_char(c, color);
}

static void scroll_down(tty_descriptor_t desc)
{
	struct vga_state *s = desc;
	for (uint_fast16_t y = 0; y < VGA_HEIGHT - 1; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			uint_fast16_t dst_i = y * VGA_WIDTH + x;
			uint_fast16_t src_i = dst_i + VGA_WIDTH;
			s->buffer[dst_i] = s->buffer[src_i];
		}
	}
}

static void break_line(tty_descriptor_t desc)
{
	struct vga_state *s = desc;
	s->row++;
	s->col = 0;

	if (s->row >= VGA_HEIGHT) {
		scroll_down(s);
		s->row--;
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			write_char(s, ' ', s->color, x, s->row);
		}
	}
}

tty_descriptor_t tty_platform_get_descriptor(void)
{
	struct vga_state *s = &GLOBAL_STATE;
	s->row = 0;
	s->col = 0;
	s->color = vga_mix_color(VGA_DEFAULT_FG, VGA_DEFAULT_BG);
	s->buffer = (uint16_t *)HIGH(VGA_BUFFER_ADDR);

	for (uint_fast16_t y = 0; y < VGA_HEIGHT; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			write_char(s, ' ', s->color, x, y);
		}
	}

	return s;
}

void tty_write(tty_descriptor_t desc, const char *data, size_t size)
{
	struct vga_state *s = desc;
	for (size_t i = 0; i < size; i++) {
		char c = data[i];

		if (c == '\n' || s->col == VGA_WIDTH) {
			break_line(s);
		}

		if (isprint(c)) {
			write_char(s, c, s->color, s->col, s->row);
			s->col++;
		}
	}
}

void tty_writeln(tty_descriptor_t desc, const char *data, size_t size)
{
	struct vga_state *s = desc;
	tty_write(desc, data, size);
	break_line(s);
}

void tty_print(tty_descriptor_t desc, const char *str)
{
	size_t s = strlen(str);
	tty_write(desc, str, s);
}

void tty_println(tty_descriptor_t desc, const char *str)
{
	size_t s = strlen(str);
	tty_writeln(desc, str, s);
}
