#include "kernel/console.h"
#include "kernel/kernel.h"
#include "kernel/mm/addr.h"
#include "kernel/modules.h"

#include "lib/cppdefs.h"
#include "lib/cstd/ctype.h"
#include "lib/cstd/string.h"
#include "lib/utils.h"

#include <stddef.h>
#include <stdint.h>

#define VIDEO_WIDTH  80
#define VIDEO_HEIGHT 25

static struct {
        volatile uint16_t *buffer;
        uint_fast16_t row;
        uint_fast16_t col;
        uint8_t color;
} STATE;

typedef uint8_t video_char_color;

enum video_color {
        VIDEO_BLACK = 0,
        VIDEO_BLUE,
        VIDEO_GREEN,
        VIDEO_CYAN,
        VIDEO_RED,
        VIDEO_MAGENTA,
        VIDEO_BROWN,
        VIDEO_LGRAY,
        VIDEO_DGRAY,
        VIDEO_LBLUE,
        VIDEO_LGREEN,
        VIDEO_LCYAN,
        VIDEO_LRED,
        VIDEO_LMAGENTA,
        VIDEO_YELLOW,
        VIDEO_WHITE
};

#define VIDEO_DEFAULT_FG VIDEO_WHITE
#define VIDEO_DEFAULT_BG VIDEO_BLACK

static inline video_char_color video_mix_color(enum video_color fg, enum video_color bg)
{
        return ((video_char_color)(fg | bg << 4));
}

static inline uint16_t video_char(unsigned char uc, video_char_color color)
{
        return ((uint16_t)uc | (uint16_t)(color << 8));
}

static void write_char(uint8_t c, video_char_color color, uint_fast16_t x, uint_fast16_t y)
{
        const uint_fast16_t index = y * VIDEO_WIDTH + x;
        STATE.buffer[index] = video_char(c, color);
}

static void scroll_down(void)
{
        for (uint_fast16_t y = 0; y < VIDEO_HEIGHT - 1; y++) {
                for (uint_fast16_t x = 0; x < VIDEO_WIDTH; x++) {
                        uint_fast16_t dst_i = y * VIDEO_WIDTH + x;
                        uint_fast16_t src_i = dst_i + VIDEO_WIDTH;
                        STATE.buffer[dst_i] = STATE.buffer[src_i];
                }
        }
}

static void break_line(void)
{
        STATE.row++;
        STATE.col = 0;

        if (STATE.row >= VIDEO_HEIGHT) {
                scroll_down();
                STATE.row--;
                for (uint_fast16_t x = 0; x < VIDEO_WIDTH; x++) {
                        write_char(' ', STATE.color, x, STATE.row);
                }
        }
}

void video_clear(struct console *c __unused)
{
        for (uint_fast16_t y = 0; y < VIDEO_HEIGHT; y++) {
                for (uint_fast16_t x = 0; x < VIDEO_WIDTH; x++) {
                        write_char(' ', STATE.color, x, y);
                }
        }
}

void video_write(struct console *c __unused, const char *restrict str, size_t size)
{
        while (size > 0) {
                const char *nl_pos = kstrchr(str, '\n');
                size_t w = MIN(size, VIDEO_WIDTH - STATE.col);
                if (nl_pos) {
                        w = MIN(w, nl_pos - str);
                }
                for (size_t i = 0; i < w; i++, str++) {
                        write_char(*str, STATE.color, STATE.col, STATE.row);
                        STATE.col++;
                }
                size -= w;
                if (size != 0) {
                        if (*str == '\n') {
                                str++;
                                size--;
                        }
                        break_line();
                }
        }
}

struct console video_console = (struct console)
{
        .name = "video_console",
#if 0
        .write = video_write,
        .clear = video_clear,
#endif
};

static bool video_available(void)
{
#ifdef __i386__
        return (true);
#else
        return (false);
#endif
}

static void video_load(void)
{
        STATE.row = 0;
        STATE.col = 0;
        STATE.color = video_mix_color(VIDEO_DEFAULT_FG, VIDEO_DEFAULT_BG);
#if 0
        STATE.buffer = VIDEO_BUFFER_ADDR;

        video_clear(c);
#endif
}

static void video_unload(void)
{
#if 0
        video_clear(c);
#endif
}

struct module i686_video = (struct module){
        .name = "i686_video",
        .fns = {
                .available = video_available,
                .load = video_load,
                .unload = video_unload,
        },
};
