#include <stdint.h>
#include <stddef.h>

#include <lib/ctype.h>
#include <lib/string.h>
#include <arch_i686/platform.h>
#include <arch_i686/vm.h>
#include <kernel/console.h>
#include <kernel/cppdefs.h>
#include <kernel/elflist.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_ADDR (0xb8000U)

struct {
	volatile uint16_t *buffer;
	uint_fast16_t row;
	uint_fast16_t col;
	uint8_t color;
} STATE;

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

static void write_char(uint8_t c, vga_char_color color, uint_fast16_t x, uint_fast16_t y)
{
	const uint_fast16_t index = y * VGA_WIDTH + x;
	STATE.buffer[index] = vga_char(c, color);
}

static void scroll_down(void)
{
	for (uint_fast16_t y = 0; y < VGA_HEIGHT - 1; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			uint_fast16_t dst_i = y * VGA_WIDTH + x;
			uint_fast16_t src_i = dst_i + VGA_WIDTH;
			STATE.buffer[dst_i] = STATE.buffer[src_i];
		}
	}
}

static void break_line(void)
{
	STATE.row++;
	STATE.col = 0;

	if (STATE.row >= VGA_HEIGHT) {
		scroll_down();
		STATE.row--;
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			write_char(' ', STATE.color, x, STATE.row);
		}
	}
}

static int vga_init(struct console *c __unused)
{
	STATE.row = 0;
	STATE.col = 0;
	STATE.color = vga_mix_color(VGA_DEFAULT_FG, VGA_DEFAULT_BG);
	STATE.buffer = (uint16_t *)HIGH(VGA_BUFFER_ADDR);

	for (uint_fast16_t y = 0; y < VGA_HEIGHT; y++) {
		for (uint_fast16_t x = 0; x < VGA_WIDTH; x++) {
			write_char(' ', STATE.color, x, y);
		}
	}

	return CONSRC_OK;
}

static int min(int x, int y)
{
	return (x < y ? x : y);
}

void vga_write(struct console *c __unused, const char *restrict data, size_t size)
{
	while (size > 0) {
		const char *nl_pos = strchr(data, '\n');
		size_t w = min(size, VGA_WIDTH - STATE.col);
		if (nl_pos) {
			w = min(w, nl_pos - data);
		}
		for (size_t i = 0; i < w; i++, data++) {
			write_char(*data, STATE.color, STATE.col, STATE.row);
			STATE.col++;
		}
		size -= w;
		if (size != 0) {
			if (*data == '\n') {
				data++;
				size--;
			}
			break_line();
		}
	}
}

struct console vga_console = (struct console){
	.name = "vga_console",
	.init = vga_init,
	.write = vga_write,
	.flags = CONSFLAG_EARLY
};
ELFLIST_NEWDATA(consoles, vga_console);
