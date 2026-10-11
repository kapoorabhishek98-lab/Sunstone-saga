/* platform.c - video setup, frame pacing, input, OAM shadow, fades, RNG */
#include "game.h"
#include "assets.h"

u16 g_keys, g_pressed, g_repeat;
u32 g_frame;
OamEntry g_oam[128];
s16 g_bg1x, g_bg1y;
static u16 keys_prev;
static u8  repeat_timer;
static u32 rng_state = 0x2545F491u;

void copy32(void *dst, const void *src, u32 words)
{
    vu32 *d = (vu32 *)dst;
    const u32 *s = (const u32 *)src;
    while (words--) *d++ = *s++;
}

void fill16(u16 *dst, u16 val, u32 count)
{
    vu16 *d = (vu16 *)dst;
    while (count--) *d++ = val;
}

static void vsync(void)
{
#ifdef HOST
    host_vsync();
#else
    while (REG_VCOUNT >= 160) {}
    while (REG_VCOUNT < 160) {}
#endif
}

void oam_hide_all(void)
{
    for (int i = 0; i < 128; i++) {
        g_oam[i].attr0 = ATTR0_HIDE;
        g_oam[i].attr1 = 0;
        g_oam[i].attr2 = 0;
    }
}

void frame_end(void)
{
    vsync();
    copy32(OAM_MEM, g_oam, sizeof(g_oam) / 4);
    REG_BG1HOFS = (u16)g_bg1x;
    REG_BG1VOFS = (u16)g_bg1y;
    audio_tick();

    u16 cur = (u16)(~REG_KEYINPUT) & KEY_MASK;
    g_pressed = cur & (u16)~keys_prev;
    g_repeat = g_pressed;
    if (g_pressed & KEY_DIRS) {
        repeat_timer = 18;
    } else if (cur & KEY_DIRS) {
        if (repeat_timer && --repeat_timer == 0) {
            g_repeat |= cur & KEY_DIRS;
            repeat_timer = 5;
        }
    }
    keys_prev = cur;
    g_keys = cur;
    g_frame++;
}

void wait_frames(int n)
{
    while (n-- > 0) frame_end();
}

void set_black(void)
{
    REG_BLDCNT = 0x3F | (3 << 6);     /* all layers + backdrop, fade to black */
    REG_BLDY = 16;
}

void fade_out(void)
{
    REG_BLDCNT = 0x3F | (3 << 6);
    for (int i = 0; i <= 16; i += 2) { REG_BLDY = i; frame_end(); }
}

void fade_in(void)
{
    REG_BLDCNT = 0x3F | (3 << 6);
    for (int i = 16; i >= 0; i -= 2) { REG_BLDY = i; frame_end(); }
    REG_BLDCNT = 0;
}

u32 rng(void)
{
    u32 x = rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return rng_state = x;
}

void rng_seed(u32 s) { rng_state = s ? s : 0x2545F491u; }

void video_init(void)
{
    REG_DISPCNT = DCNT_BLANK;
    set_black();
    REG_BG0CNT = BG_PRIO(0) | BG_CBB(0) | BG_SBB(30) | BG_SIZE_32x32;
    REG_BG1CNT = BG_PRIO(2) | BG_CBB(1) | BG_SBB(26) | BG_SIZE_64x64;
    REG_BG0HOFS = 0; REG_BG0VOFS = 0;

    copy32(CHARBLOCK(0), gfx_ui_tiles, UI_TILE_COUNT * 8);
    copy32(OBJ_VRAM, gfx_obj_tiles, OBJ_TILE_COUNT * 8);
    for (int i = 0; i < 64; i++) BG_PAL[i] = pal_ui[i];
    for (int i = 0; i < 16; i++) {
        BG_PAL[PB_EXT * 16 + i]   = pal_ext[i];
        BG_PAL[PB_INT * 16 + i]   = pal_int[i];
        BG_PAL[PB_TITLE * 16 + i] = pal_title[i];
    }
    for (int i = 0; i < OBJ_PAL_COUNT * 16; i++) OBJ_PAL[i] = pal_obj[i];

    fill16(SCREENBLOCK(30), 0, 32 * 32);
    oam_hide_all();
    copy32(OAM_MEM, g_oam, sizeof(g_oam) / 4);
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
}
