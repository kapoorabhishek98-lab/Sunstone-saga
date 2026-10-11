/* menu.c - title screen, options, field menu, status and item screens */
#include "game.h"
#include "assets.h"

/* ------------------------------------------------------------ options */
static const char * const speed_names[] = { "Slow", "Normal", "Fast", "Instant" };

static void options_draw(int sel)
{
    txt_box(3, 4, 24, 11);
    txt_print(5, 5, "OPTIONS", PB_UI_GOLD);
    txt_print(5, 7, "Text speed", PB_UI);
    txt_print(5, 9, "Sound", PB_UI);
    txt_print(5, 11, "Done", PB_UI);
    txt_print(16, 7, "<", PB_UI_GOLD);
    txt_print(17, 7, speed_names[g_game.settings.text_speed], PB_UI);
    txt_print(25, 7, ">", PB_UI_GOLD);
    txt_print(16, 9, "<", PB_UI_GOLD);
    txt_print(17, 9, g_game.settings.sound_on ? "On" : "Off", PB_UI);
    txt_print(25, 9, ">", PB_UI_GOLD);
    txt_print(5, 13, "Left/Right to change", PB_UI_GREY);
    for (int i = 0; i < 3; i++) txt_putc(4, 7 + i * 2, i == sel ? 127 : ' ', PB_UI_GOLD);
}

void options_run(void)
{
    int sel = 0;
    g_pressed = 0;
    for (;;) {
        options_draw(sel);
        frame_end();
        if (g_repeat & KEY_UP)   { sel = (sel + 2) % 3; sfx_play(SFX_CURSOR); }
        if (g_repeat & KEY_DOWN) { sel = (sel + 1) % 3; sfx_play(SFX_CURSOR); }
        int delta = (g_repeat & KEY_RIGHT) ? 1 : ((g_repeat & KEY_LEFT) ? -1 : 0);
        if (delta && sel == 0) {
            g_game.settings.text_speed = (u8)((g_game.settings.text_speed + 4 + delta) % 4);
            sfx_play(SFX_CURSOR);
        }
        if (delta && sel == 1) {
            g_game.settings.sound_on ^= 1;
            sfx_play(SFX_CURSOR);
        }
        if ((g_pressed & KEY_B) || ((g_pressed & KEY_A) && sel == 2)) {
            sfx_play(SFX_CANCEL);
            txt_clear_rect(3, 4, 24, 11);
            return;
        }
    }
}

/* ------------------------------------------------------------ title */
static void title_draw(void)
{
    copy32(CHARBLOCK(1), gfx_title_tiles, TITLE_TILE_COUNT * 8);
    fill16(SCREENBLOCK(26), 0, 4 * 1024);
    for (int y = 0; y < 20; y++)
        for (int x = 0; x < 30; x++)
            SCREENBLOCK(26)[y * 32 + x] = gfx_title_map[y * 30 + x];
    g_bg1x = 0; g_bg1y = 0;
    oam_hide_all();
    txt_clear();
    txt_print(22, 19, "M1 build", PB_UI_NARR);
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
}

int title_run(void)
{
    static const char * const items[] = { "New Game", "Continue", "Options" };
    title_draw();
    music_play(MUS_TITLE);
    fade_in();
    int sel = 0;
    for (;;) {
        /* Continue stays disabled until the SRAM save system lands (Milestone 3) */
        sel = menu_select(9, 13, 12, items, 3, sel, 0, 1u << 1);
        if (sel == 0) { rng_seed(g_frame * 2654435761u); return 0; }
        txt_clear_rect(9, 13, 12, 7);
        options_run();
    }
}

/* ------------------------------------------------------------ status */
static void stat_line(int x, int y, const char *label, u32 v)
{
    txt_print(x, y, label, PB_UI);
    txt_num(x + 10, y, v, 3, PB_UI);
}

static void status_screen(void)
{
    const PartyMember *m = &g_game.party[0];
    const CharacterDef *d = &g_characters[m->chr];
    txt_box(0, 0, 30, 20);
    int n = txt_print(2, 1, "$", PB_UI_GOLD);
    txt_print(3 + n, 1, d->title, PB_UI_GREY);
    txt_print(2, 3, "Level", PB_UI);   txt_num(8, 3, m->level, 2, PB_UI);
    txt_print(13, 3, "Next", PB_UI);
    u32 next = m->level < MAX_LEVEL ? g_exp_table[m->level + 1] - m->exp : 0;
    txt_num(18, 3, next, 6, PB_UI);
    txt_print(2, 5, "HP", PB_UI);
    txt_num(5, 5, m->hp, 3, PB_UI); txt_putc(8, 5, '/', PB_UI); txt_num(9, 5, m->max_hp, 3, PB_UI);
    txt_print(15, 5, "MP", PB_UI);
    txt_num(18, 5, m->mp, 3, PB_UI); txt_putc(21, 5, '/', PB_UI); txt_num(22, 5, m->max_mp, 3, PB_UI);
    stat_line(2, 8, "Strength", m->str);
    stat_line(2, 10, "Defense", m->def);
    stat_line(2, 12, "Intellect", m->intl);
    stat_line(16, 8, "Agility", m->agi);
    stat_line(16, 10, "Luck", m->luk);
    txt_print(16, 12, "Gold", PB_UI); txt_num(21, 12, g_game.gold, 5, PB_UI);
    txt_print(2, 15, "Spells:", PB_UI_GOLD);
    int col = 10, any = 0;
    for (int i = 0; i < d->learn_count; i++) {
        const LearnEntry *le = &d->learn[i];
        if (le->level && le->level <= m->level) {
            col += txt_print(col, 15, g_spells[le->spell].name, PB_UI) + 1;
            any = 1;
        }
    }
    if (!any) txt_print(10, 15, "none yet", PB_UI_GREY);
    txt_print(2, 18, "B: Back", PB_UI_GREY);
    g_pressed = 0;
    do { frame_end(); } while (!(g_pressed & (KEY_A | KEY_B)));
    sfx_play(SFX_CANCEL);
    txt_clear();
}

/* ------------------------------------------------------------ items */
static void items_screen(void)
{
    int list[MAX_INVENTORY], n = 0;
    for (int i = 0; i < MAX_INVENTORY; i++)
        if (g_game.inv[i].item) list[n++] = i;
    txt_box(0, 0, 30, 14);
    txt_box(0, 14, 30, 6);
    txt_print(2, 1, "ITEMS", PB_UI_GOLD);
    if (!n) {
        txt_print(2, 3, "Your bag is empty.", PB_UI_GREY);
        g_pressed = 0;
        do { frame_end(); } while (!(g_pressed & (KEY_A | KEY_B)));
        txt_clear();
        return;
    }
    int sel = 0, top = 0;
    const int rows = 5;
    g_pressed = 0;
    for (;;) {
        if (sel < top) top = sel;
        if (sel >= top + rows) top = sel - rows + 1;
        for (int j = 3; j < 13; j++)
            for (int i = 1; i < 29; i++) txt_putc(i, j, ' ', PB_UI);
        for (int r = 0; r < rows && top + r < n; r++) {
            const InvSlot *s = &g_game.inv[list[top + r]];
            const ItemDef *it = &g_items[s->item];
            int y = 3 + r * 2;
            txt_putc(1, y, top + r == sel ? 127 : ' ', PB_UI_GOLD);
            txt_print(2, y, it->name, PB_UI);
            if (it->flags & IFLAG_KEY) txt_print(24, y, "KEY", PB_UI_GOLD);
            else { txt_putc(24, y, 'x', PB_UI); txt_num(25, y, s->qty, 2, PB_UI); }
        }
        if (top > 0) txt_putc(28, 3, '^', PB_UI_GOLD);
        if (top + rows < n) txt_putc(28, 11, 'v', PB_UI_GOLD);
        for (int j = 15; j < 19; j++)
            for (int i = 1; i < 29; i++) txt_putc(i, j, ' ', PB_UI);
        txt_wrap(2, 15, 26, g_items[g_game.inv[list[sel]].item].desc, PB_UI, 2);
        frame_end();
        if ((g_repeat & KEY_UP) && sel > 0)       { sel--; sfx_play(SFX_CURSOR); }
        if ((g_repeat & KEY_DOWN) && sel < n - 1) { sel++; sfx_play(SFX_CURSOR); }
        if (g_pressed & (KEY_A | KEY_B)) { sfx_play(SFX_CANCEL); break; }
    }
    txt_clear();
}

/* ------------------------------------------------------------ field menu */
static void party_panel(void)
{
    const PartyMember *m = &g_game.party[0];
    txt_box(0, 0, 13, 7);
    txt_print(2, 1, "$", PB_UI_GOLD);
    txt_print(2, 3, "HP", PB_UI); txt_num(5, 3, m->hp, 3, PB_UI);
    txt_putc(8, 3, '/', PB_UI); txt_num(9, 3, m->max_hp, 3, PB_UI);
    txt_print(2, 5, "MP", PB_UI); txt_num(5, 5, m->mp, 3, PB_UI);
    txt_putc(8, 5, '/', PB_UI); txt_num(9, 5, m->max_mp, 3, PB_UI);
    txt_print(9, 1, "L", PB_UI_GREY); txt_num(10, 1, m->level, 2, PB_UI);
    txt_box(0, 7, 13, 3);
    txt_print(2, 8, "G", PB_UI_GOLD); txt_num(4, 8, g_game.gold, 7, PB_UI);
}

void field_menu_run(void)
{
    static const char * const items[] = { "Status", "Items", "Options", "Save", "Close" };
    int sel = 0;
    sfx_play(SFX_CONFIRM);
    for (;;) {
        party_panel();
        sel = menu_select(20, 0, 10, items, 5, sel, 1, 0);
        if (sel < 0 || sel == 4) break;
        txt_clear();
        switch (sel) {
        case 0: status_screen(); break;
        case 1: items_screen(); break;
        case 2: options_run(); break;
        case 3: dialog_run("The save journal arrives in Milestone 3.", 0); break;
        }
    }
    txt_clear();
}
