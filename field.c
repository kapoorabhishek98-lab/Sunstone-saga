/* field.c - map rendering, camera, player/NPC movement, interaction, warps */
#include "game.h"
#include "assets.h"

#define MAX_NPCS 16
#define MAP_SB 26

typedef struct {
    s16 px, py;            /* pixel position of the actor's tile (top-left) */
    u8  tx, ty;            /* logical tile (destination while moving) */
    u8  ox, oy;            /* origin tile while moving */
    u8  dir, moving, speed, alt;
    u8  sheet, pal;
    u16 timer;
    u8  home_x, home_y;
    const NpcDef *def;
} Actor;

static const s8 DX[4] = { 0, 0, -1, 1 };
static const s8 DY[4] = { 1, -1, 0, 0 };
static const u8 OPPOSITE[4] = { DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT };

static const MapDef *map;
static int map_id;
static Actor player;
static Actor npcs[MAX_NPCS];
static int npc_count;
static s16 cam_x, cam_y;

/* ------------------------------------------------------------ map */
static void bg_set(int x, int y, u16 v)
{
    int sb = (x >> 5) + ((y >> 5) << 1);
    SCREENBLOCK(MAP_SB)[sb * 1024 + (y & 31) * 32 + (x & 31)] = v;
}

static int tile_id(int x, int y)
{
    if (x < 0 || y < 0 || x >= map->w || y >= map->h) return MT_VOID;
    return map->tiles[y * map->w + x];
}

static int tile_flags(int x, int y) { return metatile_flags[tile_id(x, y)]; }

static void draw_map(void)
{
    copy32(CHARBLOCK(1), gfx_map_tiles, MAP_TILE_COUNT * 8);
    u16 void_se = metatile_se[MT_VOID * 4];
    fill16(SCREENBLOCK(MAP_SB), void_se, 4 * 1024);
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            const u16 *se = &metatile_se[map->tiles[y * map->w + x] * 4];
            bg_set(x * 2,     y * 2,     se[0]);
            bg_set(x * 2 + 1, y * 2,     se[1]);
            bg_set(x * 2,     y * 2 + 1, se[2]);
            bg_set(x * 2 + 1, y * 2 + 1, se[3]);
        }
    }
}

/* ------------------------------------------------------------ actors */
static void actor_place(Actor *a, int x, int y, int dir)
{
    a->tx = a->ox = (u8)x; a->ty = a->oy = (u8)y;
    a->px = (s16)(x * 16); a->py = (s16)(y * 16);
    a->dir = (u8)dir; a->moving = 0;
}

static int occupies(const Actor *a, int x, int y)
{
    if (a->tx == x && a->ty == y) return 1;
    return a->moving && a->ox == x && a->oy == y;
}

static Actor *npc_at(int x, int y)
{
    for (int i = 0; i < npc_count; i++)
        if (occupies(&npcs[i], x, y)) return &npcs[i];
    return 0;
}

static const WarpDef *warp_at(int x, int y)
{
    for (int i = 0; i < map->warp_count; i++)
        if (map->warps[i].x == x && map->warps[i].y == y) return &map->warps[i];
    return 0;
}

static const EventDef *event_at(int x, int y)
{
    for (int i = 0; i < map->event_count; i++)
        if (map->events[i].x == x && map->events[i].y == y) return &map->events[i];
    return 0;
}

static int blocked(int x, int y, const Actor *self)
{
    if (tile_flags(x, y) & MTF_SOLID) return 1;
    if (self != &player && occupies(&player, x, y)) return 1;
    for (int i = 0; i < npc_count; i++)
        if (&npcs[i] != self && occupies(&npcs[i], x, y)) return 1;
    return 0;
}

static void actor_start_move(Actor *a, int dir, int speed)
{
    a->dir = (u8)dir;
    a->ox = a->tx; a->oy = a->ty;
    a->tx = (u8)(a->tx + DX[dir]);
    a->ty = (u8)(a->ty + DY[dir]);
    a->moving = 1;
    a->speed = (u8)speed;
}

/* Advances a moving actor; returns 1 on the frame it arrives. */
static int actor_step(Actor *a)
{
    if (!a->moving) return 0;
    a->px = (s16)(a->px + DX[a->dir] * a->speed);
    a->py = (s16)(a->py + DY[a->dir] * a->speed);
    if (a->px == a->tx * 16 && a->py == a->ty * 16) {
        a->moving = 0;
        a->ox = a->tx; a->oy = a->ty;
        a->alt ^= 1;
        return 1;
    }
    return 0;
}

static int actor_frame(const Actor *a)
{
    int base = a->dir == DIR_DOWN ? 0 : (a->dir == DIR_UP ? 3 : 6);
    if (!a->moving) return base;
    int progress = (a->px - a->ox * 16) + (a->py - a->oy * 16);
    if (progress < 0) progress = -progress;
    if (progress >= 8) return base;
    return base + (a->alt ? 1 : 2);
}

/* ------------------------------------------------------------ camera & sprites */
static void update_camera(void)
{
    int mw = map->w * 16, mh = map->h * 16;
    int cx = player.px + 8 - 120, cy = player.py + 8 - 80;
    if (mw <= 240) cx = (mw - 240) / 2;
    else { if (cx < 0) cx = 0; if (cx > mw - 240) cx = mw - 240; }
    if (mh <= 160) cy = (mh - 160) / 2;
    else { if (cy < 0) cy = 0; if (cy > mh - 160) cy = mh - 160; }
    cam_x = (s16)cx; cam_y = (s16)cy;
    g_bg1x = cam_x; g_bg1y = cam_y;
}

static void draw_sprites(void)
{
    Actor *list[MAX_NPCS + 1];
    int n = 0;
    list[n++] = &player;
    for (int i = 0; i < npc_count; i++) list[n++] = &npcs[i];
    /* sort by y descending: lower on screen gets lower OAM index = drawn on top */
    for (int i = 1; i < n; i++) {
        Actor *k = list[i]; int j = i - 1;
        while (j >= 0 && list[j]->py < k->py) { list[j + 1] = list[j]; j--; }
        list[j + 1] = k;
    }
    oam_hide_all();
    for (int i = 0; i < n; i++) {
        Actor *a = list[i];
        int sx = a->px - cam_x, sy = a->py - cam_y - 16;
        if (sx <= -16 || sx >= 240 || sy <= -32 || sy >= 160) continue;
        int tile = (a->sheet * SPRITE_FRAMES_PER_SHEET + actor_frame(a)) * SPRITE_TILES_PER_FRAME;
        g_oam[i].attr0 = (u16)((sy & 0xFF) | ATTR0_TALL);
        g_oam[i].attr1 = (u16)((sx & 0x1FF) | ATTR1_SIZE_16x32 | (a->dir == DIR_LEFT ? ATTR1_HFLIP : 0));
        g_oam[i].attr2 = (u16)(tile | (1 << 10) | (a->pal << 12));
    }
}

static void field_draw(void)
{
    update_camera();
    draw_sprites();
}

/* ------------------------------------------------------------ loading */
static void load_map(int id)
{
    map_id = id;
    map = &g_maps[id];
    draw_map();
    npc_count = map->npc_count > MAX_NPCS ? MAX_NPCS : map->npc_count;
    for (int i = 0; i < npc_count; i++) {
        const NpcDef *d = &map->npcs[i];
        Actor *a = &npcs[i];
        actor_place(a, d->x, d->y, d->dir);
        a->sheet = d->sheet; a->pal = d->pal; a->def = d;
        a->home_x = d->x; a->home_y = d->y;
        a->timer = (u16)(30 + rng() % 90);
        a->alt = 0;
    }
}

void field_enter(int id, int x, int y, int dir)
{
    load_map(id);
    actor_place(&player, x, y, dir);
    player.sheet = SHEET_ROWAN; player.pal = OPAL_ROWAN;
    g_game.map = (u8)id; g_game.x = (u8)x; g_game.y = (u8)y; g_game.dir = (u8)dir;
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;
    field_draw();
}

/* ------------------------------------------------------------ scripts */
static int pick_page(const Script *s)
{
    int best = 0;
    for (int i = 0; i < s->count; i++)
        if (s->pages[i].cond == FLAG_NONE || flag_get(s->pages[i].cond)) best = i;
    return best;
}

static void do_inn(const char *offer)
{
    dialog_run(offer, DLG_KEEP_OPEN);
    if (!ask_yes_no()) {
        dialog_run("Bram: Door's always open, $.", 0);
        return;
    }
    dialog_close();
    fade_out();
    sfx_play(SFX_REST);
    party_heal_all();
    wait_frames(50);
    if (flag_get(FLAG_FOUND_SHARD) && !flag_get(FLAG_M1_COMPLETE)) {
        u16 saved = REG_DISPCNT;
        oam_hide_all();
        REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
        narration(g_dream_pages, 0);
        flag_set(FLAG_M1_COMPLETE);
        REG_DISPCNT = saved;
        field_draw();
        fade_in();
        dialog_run("Bram: Morning, $! You were talking in your sleep.\f"
                   "Something about invaders?\fHuh. Breakfast's on the house too.", 0);
        return;
    }
    field_draw();
    fade_in();
    dialog_run("Bram: Morning! You look a sight better, $.", 0);
}

static void run_script(const Script *s)
{
    if (!s->count) return;
    const Page *pg = &s->pages[pick_page(s)];
    if (pg->action == ACT_INN) do_inn(pg->text);
    else dialog_run(pg->text, 0);
    flag_set(pg->set);
    if (pg->action == ACT_GET_STAR_SHARD) inv_add(ITEM_STAR_SHARD, 1);
}

static void examine_tile(int x, int y)
{
    switch (tile_id(x, y)) {
    case MT_RUBBLE: dialog_run("Charred beams and split stone. This was the old mill.", 0); break;
    case MT_WATER:  dialog_run("The pond is still and very cold.", 0); break;
    case MT_BED:    dialog_run("A clean bed with a patched quilt. Bram is the one to ask.", 0); break;
    case MT_SHELF:  dialog_run("Jars of pickled things. Some are labeled. Most are not.", 0); break;
    default: break;
    }
}

static void interact(void)
{
    int fx = player.tx + DX[player.dir], fy = player.ty + DY[player.dir];
    Actor *n = npc_at(fx, fy);
    if (!n && (tile_flags(fx, fy) & MTF_COUNTER))
        n = npc_at(fx + DX[player.dir], fy + DY[player.dir]);
    if (n && !n->moving) {
        n->dir = OPPOSITE[player.dir];
        field_draw();
        run_script(&n->def->script);
        n->timer = 90;
        return;
    }
    const EventDef *e = event_at(fx, fy);
    if (e) { run_script(&e->script); return; }
    examine_tile(fx, fy);
}

static void take_warp(const WarpDef *w)
{
    sfx_play(SFX_DOOR);
    fade_out();
    field_enter(w->map, w->dx, w->dy, w->ddir);
    fade_in();
}

/* ------------------------------------------------------------ update */
static void update_npcs(void)
{
    for (int i = 0; i < npc_count; i++) {
        Actor *a = &npcs[i];
        if (a->moving) { actor_step(a); continue; }
        if (a->def->move != MOVE_WANDER) continue;
        if (a->timer) { a->timer--; continue; }
        a->timer = (u16)(60 + rng() % 150);
        int d = (int)(rng() & 3);
        int nx = a->tx + DX[d], ny = a->ty + DY[d];
        a->dir = (u8)d;
        int dx = nx - a->home_x, dy = ny - a->home_y;
        if (dx < -2 || dx > 2 || dy < -2 || dy > 2) continue;
        if (blocked(nx, ny, a) || warp_at(nx, ny) || event_at(nx, ny)) continue;
        actor_start_move(a, d, 1);
    }
}

static void update_player(void)
{
    if (player.moving) {
        if (actor_step(&player)) {
            g_game.x = player.tx; g_game.y = player.ty;
            const WarpDef *w = warp_at(player.tx, player.ty);
            if (w) { take_warp(w); return; }
        } else {
            return;
        }
    }
    if (g_pressed & KEY_START) {
        field_menu_run();
        field_draw();
        return;
    }
    if (g_pressed & KEY_A) { interact(); return; }

    int d = -1;
    if (g_keys & KEY_UP) d = DIR_UP;
    else if (g_keys & KEY_DOWN) d = DIR_DOWN;
    else if (g_keys & KEY_LEFT) d = DIR_LEFT;
    else if (g_keys & KEY_RIGHT) d = DIR_RIGHT;
    if (d < 0) return;
    player.dir = (u8)d;
    g_game.dir = (u8)d;
    int nx = player.tx + DX[d], ny = player.ty + DY[d];
    if (!blocked(nx, ny, &player))
        actor_start_move(&player, d, (g_keys & KEY_B) ? 2 : 1);
}

void field_run(void)
{
    for (;;) {
        update_player();
        update_npcs();
        field_draw();
        g_game.play_frames++;
        frame_end();
    }
}
