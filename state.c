/* state.c - game state, story flags, inventory, party stats */
#include "game.h"

GameState g_game;

int flag_get(int f) { return (g_game.flags[f >> 5] >> (f & 31)) & 1; }
void flag_set(int f) { if (f) g_game.flags[f >> 5] |= 1u << (f & 31); }

static u16 grow(u16 base, u8 g10, int level) { return (u16)(base + (g10 * (level - 1)) / 10); }

void member_init(PartyMember *m, int chr, int level)
{
    const CharacterDef *d = &g_characters[chr];
    m->chr = (u8)chr;
    m->level = (u8)level;
    m->exp = g_exp_table[level];
    m->max_hp = grow(d->hp, d->g_hp, level);
    m->max_mp = grow(d->mp, d->g_mp, level);
    m->str  = (u8)grow(d->str, d->g_str, level);
    m->def  = (u8)grow(d->def, d->g_def, level);
    m->intl = (u8)grow(d->intl, d->g_int, level);
    m->agi  = (u8)grow(d->agi, d->g_agi, level);
    m->luk  = (u8)grow(d->luk, d->g_luk, level);
    m->hp = m->max_hp;
    m->mp = m->max_mp;
    m->status = 0;
    for (int i = 0; i < SLOT_COUNT; i++) m->equip[i] = 0;
}

/* Equipment bonuses are applied on top of base stats wherever stats are read;
 * this clamps current HP/MP after any change. */
void party_recalc(PartyMember *m)
{
    if (m->hp > m->max_hp) m->hp = m->max_hp;
    if (m->mp > m->max_mp) m->mp = m->max_mp;
}

void party_heal_all(void)
{
    for (int i = 0; i < g_game.party_count; i++) {
        PartyMember *m = &g_game.party[i];
        m->hp = m->max_hp;
        m->mp = m->max_mp;
        m->status = 0;
    }
}

/* Returns 1 on success, 0 if the bag is full. Key items never stack. */
int inv_add(int item, int qty)
{
    int key = g_items[item].flags & IFLAG_KEY;
    for (int i = 0; i < MAX_INVENTORY && !key; i++) {
        if (g_game.inv[i].item == item && g_game.inv[i].qty + qty <= 99) {
            g_game.inv[i].qty = (u8)(g_game.inv[i].qty + qty);
            return 1;
        }
    }
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (g_game.inv[i].item == ITEM_NONE) {
            g_game.inv[i].item = (u8)item;
            g_game.inv[i].qty = (u8)qty;
            return 1;
        }
    }
    return 0;
}

void settings_default(void)
{
    g_game.settings.text_speed = TEXT_NORMAL;
    g_game.settings.sound_on = 1;
    g_game.settings.music_vol = 8;
    g_game.settings.sfx_vol = 8;
    g_game.settings.battle_speed = 1;
}

/* Starts a fresh adventure; keeps the player's current option settings. */
void game_new(void)
{
    Settings keep = g_game.settings;
    u8 *p = (u8 *)&g_game;
    for (u32 i = 0; i < sizeof(g_game); i++) p[i] = 0;
    const char *n = "Rowan";
    for (int i = 0; n[i]; i++) g_game.name[i] = n[i];
    member_init(&g_game.party[0], CHR_ROWAN, 1);
    g_game.party_count = 1;
    g_game.recruited = 1u << CHR_ROWAN;
    g_game.gold = 50;
    inv_add(ITEM_HEALING_HERB, 3);
    g_game.settings = keep;
}
