/* game.h - shared engine API */
#ifndef GAME_H
#define GAME_H
#include "gba.h"
#include "data.h"

/* platform.c */
extern u16 g_keys, g_pressed, g_repeat;
extern u32 g_frame;
extern OamEntry g_oam[128];
extern s16 g_bg1x, g_bg1y;
void video_init(void);
void frame_end(void);                 /* wait vblank, push OAM/scroll, poll input */
void wait_frames(int n);
void oam_hide_all(void);
void fade_out(void);
void fade_in(void);
void set_black(void);
void copy32(void *dst, const void *src, u32 words);
void fill16(u16 *dst, u16 val, u32 count);
u32  rng(void);
void rng_seed(u32 s);

/* audio.c */
enum { SFX_CURSOR, SFX_CONFIRM, SFX_CANCEL, SFX_ITEM, SFX_DOOR, SFX_REST, SFX_COUNT };
enum { MUS_NONE, MUS_TITLE, MUS_TOWN, MUS_FIELD, MUS_DUNGEON, MUS_BATTLE, MUS_BOSS };
void audio_init(void);
void audio_tick(void);
void sfx_play(int id);
void music_play(int id);              /* hook: soundtrack arrives in Milestone 7 */

/* text.c */
#define DLG_KEEP_OPEN 1
void txt_clear(void);
void txt_clear_rect(int x, int y, int w, int h);
void txt_box(int x, int y, int w, int h);
void txt_putc(int x, int y, char c, int pb);
int  txt_print(int x, int y, const char *s, int pb);
void txt_num(int x, int y, u32 v, int width, int pb);
int  txt_wrap(int x, int y, int w, const char *s, int pb, int max_lines);
void dialog_run(const char *text, int flags);
void dialog_close(void);
int  menu_select(int x, int y, int w, const char * const *items, int count,
                 int start, int cancel_ok, u32 disabled_mask);
int  ask_yes_no(void);
void narration(const char * const *pages, int skippable);

/* field.c */
void field_enter(int map, int x, int y, int dir);
void field_run(void);                 /* never returns */

/* menu.c */
int  title_run(void);                 /* returns when New Game is chosen */
void options_run(void);
void field_menu_run(void);

/* state.c */
extern GameState g_game;
void game_new(void);
void settings_default(void);
int  flag_get(int f);
void flag_set(int f);
int  inv_add(int item, int qty);
void party_recalc(PartyMember *m);
void party_heal_all(void);
void member_init(PartyMember *m, int chr, int level);

#endif
