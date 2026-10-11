/* data.h - core data models. All game content is defined as const tables
 * (data_game.c, data_maps.c) so new content never needs engine changes. */
#ifndef DATA_H
#define DATA_H
#include "gba.h"

#define MAX_PARTY      4
#define MAX_INVENTORY  40
#define NAME_LEN       8
#define MAX_LEVEL      35
#define FLAG_WORDS     8          /* 256 story flags */

/* ---------------------------------------------------------- characters */
enum { CHR_ROWAN, CHR_MIRA, CHR_GARRICK, CHR_SERA, CHR_FINN, CHR_COUNT };

enum { ELEM_NONE, ELEM_FIRE, ELEM_ICE, ELEM_LIGHTNING, ELEM_WIND, ELEM_LIGHT };
#define ELEM_BIT(e) (1u << (e))

enum {
    SPELL_EMBER, SPELL_FROST, SPELL_SPARK, SPELL_GALE,
    SPELL_MEND, SPELL_GREATER_MEND, SPELL_LIFEBLOOM, SPELL_CLEANSE, SPELL_WARD,
    SPELL_RALLY, SPELL_BREAKER, SPELL_QUICKSTEP, SPELL_DISRUPT, SPELL_TREASURE_SENSE,
    SPELL_COUNT
};
enum { TGT_ENEMY, TGT_ALL_ENEMIES, TGT_ALLY, TGT_ALL_ALLIES, TGT_SELF, TGT_FIELD };
enum { SK_MAGIC_DMG, SK_PHYS_DMG, SK_HEAL, SK_BUFF_DEF, SK_BUFF_ATK, SK_BUFF_AGI,
       SK_CURE, SK_DISABLE, SK_FIELD };

typedef struct {
    const char *name, *desc;
    u8 mp, target, kind, element;
    u16 power;
    u8 field_ok;                        /* usable from the field menu */
} SpellDef;

typedef struct { u8 spell, level; } LearnEntry;   /* level 0 = quest reward */

typedef struct {
    const char *name, *title;
    u16 hp, mp;                         /* level 1 values */
    u8  str, def, intl, agi, luk;
    u8  g_hp, g_mp, g_str, g_def, g_int, g_agi, g_luk;   /* growth, tenths/level */
    const LearnEntry *learn; u8 learn_count;
    u8  sheet, pal;
} CharacterDef;

#define STATUS_POISON  0x01
#define STATUS_SLEEP   0x02
#define STATUS_SILENCE 0x04
#define STATUS_STUN    0x08

enum { SLOT_WEAPON, SLOT_SHIELD, SLOT_HELM, SLOT_ARMOR, SLOT_ACCESSORY, SLOT_COUNT };

typedef struct {
    u8  chr, level;
    u32 exp;
    u16 hp, mp, max_hp, max_mp;
    u8  str, def, intl, agi, luk;       /* base stats before equipment */
    u8  status;
    u8  equip[SLOT_COUNT];              /* item ids, 0 = empty */
} PartyMember;

/* ---------------------------------------------------------- items */
enum { ITYPE_USE, ITYPE_WEAPON, ITYPE_SHIELD, ITYPE_HELM, ITYPE_ARMOR,
       ITYPE_ACCESSORY, ITYPE_KEY };
#define IFLAG_KEY    0x01   /* quest item: cannot be sold or dropped */
#define IFLAG_BATTLE 0x02
#define IFLAG_FIELD  0x04
enum { IEFF_NONE, IEFF_HEAL_HP, IEFF_HEAL_MP, IEFF_CURE_POISON, IEFF_CURE_ALL,
       IEFF_REVIVE, IEFF_ESCAPE_DUNGEON };
#define EQ(c) (1u << (c))           /* equip mask bit per character */
#define EQ_ALL 0x1F

enum {
    ITEM_NONE,
    ITEM_HEALING_HERB, ITEM_SILVERLEAF_TONIC, ITEM_STARDEW_VIAL, ITEM_ANTIDOTE_ROOT,
    ITEM_CLEARBELL, ITEM_WAKEFEATHER, ITEM_WAYSTONE,
    ITEM_COURIER_KNIFE, ITEM_OAK_STAFF, ITEM_IRON_SWORD, ITEM_HUNTING_BOW,
    ITEM_PLANK_SHIELD, ITEM_LEATHER_CAP, ITEM_TRAVEL_COAT, ITEM_LUCKY_BUTTON,
    ITEM_STAR_SHARD,
    ITEM_COUNT
};

typedef struct {
    const char *name, *desc;
    u16 price;
    u8  type, flags, effect;
    u16 power;
    s8  atk, def, intl, agi, luk;
    u8  equip_mask;
} ItemDef;

typedef struct { u8 item, qty; } InvSlot;

/* ---------------------------------------------------------- enemies */
enum { AI_BASIC, AI_CASTER, AI_HEALER, AI_COWARD, AI_CHARGER, AI_BOSS };
enum { ENEMY_MOSSFANG, ENEMY_BRAMBLE_HOPPER, ENEMY_GLASSWING_WISP, ENEMY_MUDSHELL,
       ENEMY_LANTERN_MIMIC, ENEMY_HOLLOW_SENTINEL, ENEMY_COUNT };
typedef struct {
    const char *name;
    u16 hp, mp;
    u8  str, def, intl, agi, luk;
    u16 exp, gold;
    u8  weak, resist;                   /* ELEM_BIT masks */
    u8  drop, drop_rate;                /* drop_rate out of 256 */
    u8  ai;
} EnemyDef;

/* ---------------------------------------------------------- maps & events */
enum { MAP_BRINDLEFORD, MAP_INN, MAP_COUNT };
enum { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT };
enum { ACT_NONE, ACT_GET_STAR_SHARD, ACT_INN };
enum { MOVE_STATIC, MOVE_WANDER };

/* A script is a list of pages; the highest page whose condition flag is set
 * (0 = always) runs. When it finishes, `set` is raised and `action` runs. */
typedef struct { u8 cond, set, action; const char *text; } Page;
typedef struct { const Page *pages; u8 count; } Script;
typedef struct { u8 x, y, sheet, pal, dir, move; Script script; } NpcDef;
typedef struct { u8 x, y; Script script; } EventDef;
typedef struct { u8 x, y, map, dx, dy, ddir; } WarpDef;
typedef struct {
    const char *name;
    u8 w, h;
    const u8 *tiles;
    const NpcDef *npcs;     u8 npc_count;
    const EventDef *events; u8 event_count;
    const WarpDef *warps;   u8 warp_count;
} MapDef;

/* In-text control code: plays the item jingle at that point of a message. */
#define TEXT_SFX_ITEM "\x01"

enum { FLAG_NONE, FLAG_TALKED_ELDER, FLAG_FOUND_SHARD, FLAG_M1_COMPLETE, FLAG_COUNT };

/* ---------------------------------------------------------- game state */
enum { TEXT_SLOW, TEXT_NORMAL, TEXT_FAST, TEXT_INSTANT };
typedef struct {
    u8 text_speed, sound_on, music_vol, sfx_vol, battle_speed;
} Settings;

typedef struct {
    char name[NAME_LEN + 1];
    PartyMember party[MAX_PARTY];
    u8  party_count;
    u8  recruited;                      /* CHR_ bitmask */
    u32 gold;
    InvSlot inv[MAX_INVENTORY];
    u32 flags[FLAG_WORDS];
    u8  map, x, y, dir;
    u32 play_frames;
    Settings settings;
} GameState;

/* SRAM save file (Milestone 3). Validated by magic, version and checksum. */
#define SAVE_MAGIC   0x53435257u  /* "WRCS" */
#define SAVE_VERSION 1
#define SAVE_SLOTS   3
typedef struct { u32 magic; u16 version, size; u32 checksum; GameState state; } SaveFile;

/* ---------------------------------------------------------- content tables */
extern const CharacterDef g_characters[CHR_COUNT];
extern const SpellDef     g_spells[SPELL_COUNT];
extern const ItemDef      g_items[ITEM_COUNT];
extern const EnemyDef     g_enemies[ENEMY_COUNT];
extern const u32          g_exp_table[MAX_LEVEL + 1];
extern const MapDef       g_maps[MAP_COUNT];
extern const char * const g_intro_pages[];
extern const char * const g_dream_pages[];

#endif
