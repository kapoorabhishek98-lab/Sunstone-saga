/* text.c - window boxes, typewriter dialogue, menus, narration (BG0) */
#include "game.h"
#include "assets.h"

#define TXT_SB SCREENBLOCK(30)
#define TILE_SPACE 32
#define TILE_MORE  9

/* dialogue box geometry: full width, rows 13..19, three double-spaced lines */
#define DLG_Y      13
#define DLG_H      7
#define DLG_COL0   2
#define DLG_COLS   26
#define DLG_LINES  3

static inline void put(int x, int y, u16 tile, int pb)
{
    if ((unsigned)x < 32 && (unsigned)y < 32)
        TXT_SB[y * 32 + x] = tile | (u16)(pb << 12);
}

void txt_clear(void) { fill16(TXT_SB, 0, 32 * 32); }

void txt_clear_rect(int x, int y, int w, int h)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) put(x + i, y + j, 0, 0);
}

void txt_box(int x, int y, int w, int h)
{
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int t = TILE_SPACE;
            int top = j == 0, bot = j == h - 1, lft = i == 0, rgt = i == w - 1;
            if (top && lft) t = 1; else if (top && rgt) t = 3; else if (top) t = 2;
            else if (bot && lft) t = 6; else if (bot && rgt) t = 8; else if (bot) t = 7;
            else if (lft) t = 4; else if (rgt) t = 5;
            put(x + i, y + j, (u16)t, PB_UI);
        }
    }
}

void txt_putc(int x, int y, char c, int pb)
{
    unsigned char u = (unsigned char)c;
    if (u < 32 || u > 127) u = '?';
    put(x, y, u, pb);
}

/* Prints a string; '$' expands to the hero's name. Returns columns used. */
int txt_print(int x, int y, const char *s, int pb)
{
    int n = 0;
    for (; *s && *s != '\n'; s++) {
        if (*s == '$') {
            for (const char *p = g_game.name; *p; p++) txt_putc(x + n++, y, *p, pb);
        } else {
            txt_putc(x + n++, y, *s, pb);
        }
    }
    return n;
}

void txt_num(int x, int y, u32 v, int width, int pb)
{
    char buf[11]; int n = 0;
    do { buf[n++] = (char)('0' + v % 10); v /= 10; } while (v && n < 10);
    for (int i = 0; i < width - n; i++) txt_putc(x + i, y, ' ', pb);
    int off = width > n ? width - n : 0;
    for (int i = 0; i < n; i++) txt_putc(x + off + i, y, buf[n - 1 - i], pb);
}

/* Word-wrapped, double-spaced print. Returns lines used. */
int txt_wrap(int x, int y, int w, const char *s, int pb, int max_lines)
{
    int line = 0, col = 0;
    while (*s && line < max_lines) {
        int len = 0;
        while (s[len] && s[len] != ' ') len++;
        if (col > 0 && col + len > w) { line++; col = 0; if (line >= max_lines) break; }
        for (int i = 0; i < len; i++) txt_putc(x + col + i, y + line * 2, s[i], pb);
        col += len;
        s += len;
        while (*s == ' ') { s++; if (col > 0 && col < w) col++; }
    }
    return line + 1;
}

/* ------------------------------------------------------------ dialogue */
static char expand_buf[640];

static void expand(const char *src)
{
    int n = 0;
    while (*src && n < (int)sizeof(expand_buf) - 1) {
        if (*src == '$') {
            for (const char *p = g_game.name; *p && n < (int)sizeof(expand_buf) - 1; p++)
                expand_buf[n++] = *p;
        } else {
            expand_buf[n++] = *src;
        }
        src++;
    }
    expand_buf[n] = 0;
}

static void dlg_clear_inner(void)
{
    for (int j = 1; j < DLG_H - 1; j++)
        for (int i = 1; i < 29; i++) put(i, DLG_Y + j, TILE_SPACE, PB_UI);
}

static void wait_confirm(int show_arrow)
{
    g_pressed = 0;
    for (;;) {
        if (show_arrow)
            put(28, DLG_Y + DLG_H - 2, (g_frame & 16) ? TILE_SPACE : TILE_MORE, PB_UI);
        frame_end();
        if (g_pressed & (KEY_A | KEY_B)) break;
    }
    if (show_arrow) put(28, DLG_Y + DLG_H - 2, TILE_SPACE, PB_UI);
    sfx_play(SFX_CURSOR);
}

static int word_len(const char *p)
{
    int n = 0;
    while (p[n] && p[n] != ' ' && p[n] != '\n' && p[n] != '\f' && p[n] != '\x01') n++;
    return n;
}

void dialog_close(void) { txt_clear_rect(0, DLG_Y, 30, DLG_H); }

void dialog_run(const char *text, int flags)
{
    expand(text);
    txt_box(0, DLG_Y, 30, DLG_H);
    g_pressed = 0;

    const char *p = expand_buf;
    int line = 0, col = 0, skip = 0, tick = 0;
    int speed = g_game.settings.text_speed;

    while (*p) {
        char c = *p;
        if (c == '\x01') { sfx_play(SFX_ITEM); p++; continue; }
        if (c == '\f') {
            wait_confirm(1); dlg_clear_inner();
            line = col = skip = 0; p++;
            continue;
        }
        int at_word_start = (p == expand_buf || p[-1] == ' ' || p[-1] == '\n' || p[-1] == '\f');
        int need_break = (c == '\n') ||
                         (c != ' ' && at_word_start && col > 0 && col + word_len(p) > DLG_COLS) ||
                         (col >= DLG_COLS);
        if (need_break) {
            line++; col = 0;
            if (line >= DLG_LINES) { wait_confirm(1); dlg_clear_inner(); line = 0; skip = 0; }
            if (c == '\n') { p++; continue; }
        }
        if (c == ' ' && col == 0) { p++; continue; }

        txt_putc(DLG_COL0 + col, DLG_Y + 1 + line * 2, c, PB_UI);
        col++; p++;

        if (!skip && speed != TEXT_INSTANT && c != ' ') {
            int frames = speed == TEXT_SLOW ? 2 : (speed == TEXT_NORMAL ? 1 : (tick++ & 1));
            while (frames-- > 0) {
                frame_end();
                if (g_pressed & (KEY_A | KEY_B)) { skip = 1; g_pressed = 0; break; }
            }
        }
    }
    if (!(flags & DLG_KEEP_OPEN)) {
        wait_confirm(1);
        dialog_close();
    }
}

/* ------------------------------------------------------------ menus */
int menu_select(int x, int y, int w, const char * const *items, int count,
                int start, int cancel_ok, u32 disabled_mask)
{
    int h = count * 2 + 1;
    txt_box(x, y, w, h);
    for (int i = 0; i < count; i++)
        txt_print(x + 2, y + 1 + i * 2, items[i],
                  (disabled_mask & (1u << i)) ? PB_UI_GREY : PB_UI);
    int sel = start;
    g_pressed = 0;
    for (;;) {
        for (int i = 0; i < count; i++)
            put(x + 1, y + 1 + i * 2, i == sel ? 127 : TILE_SPACE, PB_UI_GOLD);
        frame_end();
        if (g_repeat & KEY_UP)   { sel = (sel + count - 1) % count; sfx_play(SFX_CURSOR); }
        if (g_repeat & KEY_DOWN) { sel = (sel + 1) % count;         sfx_play(SFX_CURSOR); }
        if (g_pressed & KEY_A) {
            if (disabled_mask & (1u << sel)) { sfx_play(SFX_CANCEL); continue; }
            sfx_play(SFX_CONFIRM);
            return sel;
        }
        if ((g_pressed & KEY_B) && cancel_ok) { sfx_play(SFX_CANCEL); return -1; }
    }
}

int ask_yes_no(void)
{
    static const char * const yn[] = { "Yes", "No" };
    int r = menu_select(22, DLG_Y - 6, 8, yn, 2, 0, 1, 0);
    txt_clear_rect(22, DLG_Y - 6, 8, 5);
    return r == 0;
}

/* ------------------------------------------------------------ narration */
/* Full-screen centered text on black. Caller must have the screen faded out. */
void narration(const char * const *pages, int skippable)
{
    for (int pg = 0; pages[pg]; pg++) {
        txt_clear();
        const char *s = pages[pg];
        int lines = 1;
        for (const char *q = s; *q; q++) if (*q == '\n') lines++;
        int y = 10 - lines;                         /* double spaced, centered */
        while (*s) {
            int len = 0;
            while (s[len] && s[len] != '\n') len++;
            int x = (30 - len) / 2;
            for (int i = 0; i < len; i++) txt_putc(x + i, y, s[i], PB_UI_NARR);
            y += 2;
            s += len;
            if (*s == '\n') s++;
        }
        fade_in();
        g_pressed = 0;
        for (;;) {
            frame_end();
            if (g_pressed & (KEY_A | KEY_B)) break;
            if (skippable && (g_pressed & KEY_START)) { fade_out(); txt_clear(); return; }
        }
        sfx_play(SFX_CURSOR);
        fade_out();
    }
    txt_clear();
}
