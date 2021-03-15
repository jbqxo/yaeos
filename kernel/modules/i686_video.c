#include "kernel/console.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/addr.h"
#include "kernel/mm/dev.h"
#include "kernel/modules.h"
#include "kernel/resources.h"

#include "lib/cppdefs.h"
#include "lib/cstd/ctype.h"
#include "lib/cstd/string.h"
#include "lib/utils.h"
#include "lib/elflist.h"

#include <stddef.h>
#include <stdint.h>

#define VIDEO_WIDTH        (UINT8_C(80))
#define VIDEO_HEIGHT       (UINT8_C(25))
#define VGA3_BUFFER_OFFSET ((uintptr_t)0x18000)

static struct i686_video_state {
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
        return ((uint16_t)((color << 8) | uc));
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

static void video_clear(struct console *c)
{
        struct i686_video_state *s = c->data;

        for (uint_fast16_t y = 0; y < VIDEO_HEIGHT; y++) {
                for (uint_fast16_t x = 0; x < VIDEO_WIDTH; x++) {
                        write_char(' ', s->color, x, y);
                }
        }
}

static void video_write(struct console *restrict c, const char *str, size_t size)
{
        struct i686_video_state *state = c->data;
        while (size > 0) {
                const char *nl_pos = kstrchr(str, '\n');
                size_t w = MIN(size, VIDEO_WIDTH - state->col);
                if (nl_pos) {
                        w = MIN(w, (uintptr_t)nl_pos - (uintptr_t)str);
                }
                for (size_t i = 0; i < w; i++, str++) {
                        write_char(*str, state->color, state->col, state->row);
                        state->col++;
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

static struct console video_console = (struct console){
        .name = "video_console",
        .ops.write = video_write,
        .ops.clear = video_clear,
        .data = &STATE,
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
        struct resource *vid = resources_kclaim_by_id("platform", "video");
        if (__unlikely(NULL == vid)) {
                LOGF_P("Couldn't find platform video buffer\n");
        }

        void *vid_mem = kdev_map_resource(vid);
        if (__unlikely(NULL == vid_mem)) {
                LOGF_P("Couldn't map video memory in the virtual address space\n");
        }

        STATE.row = 0;
        STATE.col = 0;
        STATE.color = video_mix_color(VIDEO_DEFAULT_FG, VIDEO_DEFAULT_BG);
        STATE.buffer = (void *)((uintptr_t)vid_mem + VGA3_BUFFER_OFFSET);

        console_register(&video_console);
}

static void video_unload(void)
{
        kdev_unmap_resource((void *)((uintptr_t)STATE.buffer - VGA3_BUFFER_OFFSET));
}

static struct module i686_video = (struct module){
        .name = "i686_video",
        .fns = {
                .available = video_available,
                .load = video_load,
                .unload = video_unload,
        },
};
ELFLIST_NEWDATA(modules, i686_video);
