/* audio.c - square-wave sound effects on DMG channel 2.
 * Channel 1 is reserved for the music driver (Milestone 7). */
#include "game.h"

/* GBA square frequency register value: n = 2048 - 131072 / hz */
#define N_C4 1547
#define N_G4 1714
#define N_C5 1797
#define N_E5 1849
#define N_G5 1881
#define N_A5 1899
#define N_C6 1923
#define N_E6 1949

typedef struct { u16 note; u8 frames; } Note;
static const Note s_cursor[]  = { {N_C6, 3}, {0, 0} };
static const Note s_confirm[] = { {N_G5, 3}, {N_C6, 6}, {0, 0} };
static const Note s_cancel[]  = { {N_E5, 3}, {N_C5, 6}, {0, 0} };
static const Note s_item[]    = { {N_C5, 7}, {N_E5, 7}, {N_G5, 7}, {N_C6, 7}, {N_E6, 20}, {0, 0} };
static const Note s_door[]    = { {N_G4, 4}, {N_C4, 8}, {0, 0} };
static const Note s_rest[]    = { {N_C5, 10}, {N_G4, 10}, {N_A5, 10}, {N_G5, 24}, {0, 0} };
static const Note * const s_table[SFX_COUNT] = { s_cursor, s_confirm, s_cancel, s_item, s_door, s_rest };

static const Note *cur;
static u8 timer;
static int current_music;

void audio_init(void)
{
    REG_SNDSTAT = 0x80;                          /* master enable */
    REG_SNDDMGCNT = 0x77 | (0x3 << 8) | (0x3 << 12); /* full volume, ch1+ch2 both sides */
    REG_SNDDSCNT = 2;                            /* DMG channels at 100% */
    REG_SND1SWEEP = 0x0008;
}

void sfx_play(int id)
{
    if (!g_game.settings.sound_on || id < 0 || id >= SFX_COUNT) return;
    cur = s_table[id];
    timer = 0;
}

void audio_tick(void)
{
    if (!cur) return;
    if (timer) { timer--; return; }
    if (cur->frames == 0) { cur = 0; return; }
    u16 vol = (u16)(g_game.settings.sfx_vol > 15 ? 15 : (g_game.settings.sfx_vol + 4));
    if (vol > 15) vol = 15;
    REG_SND2CNT = (u16)((2 << 6) | (2 << 8) | (vol << 12));   /* 50% duty, decaying envelope */
    REG_SND2FREQ = (u16)(cur->note | 0x8000);
    timer = (u8)(cur->frames - 1);
    cur++;
}

void music_play(int id) { current_music = id; }   /* placeholder hook */
