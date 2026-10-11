/* data_game.c - characters, spells, items, enemies, experience curve */
#include "data.h"
#include "assets.h"

/* ---------------------------------------------------------- spells */
const SpellDef g_spells[SPELL_COUNT] = {
    [SPELL_EMBER]          = { "Ember",       "A lick of flame at one foe.",           3, TGT_ENEMY,       SK_MAGIC_DMG, ELEM_FIRE,      14, 0 },
    [SPELL_FROST]          = { "Frost",       "Biting cold on one foe.",               3, TGT_ENEMY,       SK_MAGIC_DMG, ELEM_ICE,       14, 0 },
    [SPELL_SPARK]          = { "Spark",       "A crack of lightning on one foe.",      4, TGT_ENEMY,       SK_MAGIC_DMG, ELEM_LIGHTNING, 18, 0 },
    [SPELL_GALE]           = { "Gale",        "Cutting wind strikes all foes.",        7, TGT_ALL_ENEMIES, SK_MAGIC_DMG, ELEM_WIND,      22, 0 },
    [SPELL_MEND]           = { "Mend",        "Restores some HP to one ally.",         3, TGT_ALLY,        SK_HEAL,      ELEM_LIGHT,     30, 1 },
    [SPELL_GREATER_MEND]   = { "Greater Mend","Restores much HP to one ally.",         8, TGT_ALLY,        SK_HEAL,      ELEM_LIGHT,     90, 1 },
    [SPELL_LIFEBLOOM]      = { "Lifebloom",   "Restores HP to the whole party.",      18, TGT_ALL_ALLIES,  SK_HEAL,      ELEM_LIGHT,    120, 1 },
    [SPELL_CLEANSE]        = { "Cleanse",     "Cures poison, sleep and silence.",      4, TGT_ALLY,        SK_CURE,      ELEM_LIGHT,      0, 1 },
    [SPELL_WARD]           = { "Ward",        "Halves damage to one ally for a time.", 4, TGT_ALLY,        SK_BUFF_DEF,  ELEM_NONE,      50, 0 },
    [SPELL_RALLY]          = { "Rally",       "Raises one ally's attack.",             4, TGT_ALLY,        SK_BUFF_ATK,  ELEM_NONE,      30, 0 },
    [SPELL_BREAKER]        = { "Breaker",     "A crushing blow. Slow but heavy.",      5, TGT_ENEMY,       SK_PHYS_DMG,  ELEM_NONE,     175, 0 },
    [SPELL_QUICKSTEP]      = { "Quickstep",   "Raises one ally's agility.",            3, TGT_ALLY,        SK_BUFF_AGI,  ELEM_NONE,      40, 0 },
    [SPELL_DISRUPT]        = { "Disrupt",     "May stun a foe or break its focus.",    4, TGT_ENEMY,       SK_DISABLE,   ELEM_NONE,      60, 0 },
    [SPELL_TREASURE_SENSE] = { "Treasure Sense", "Reveals hidden items nearby.",       2, TGT_FIELD,       SK_FIELD,     ELEM_NONE,       0, 1 },
};

/* ---------------------------------------------------------- characters */
static const LearnEntry learn_rowan[]   = { {SPELL_SPARK, 4}, {SPELL_MEND, 7}, {SPELL_RALLY, 12} };
static const LearnEntry learn_mira[]    = { {SPELL_MEND, 1}, {SPELL_WARD, 5}, {SPELL_CLEANSE, 9},
                                            {SPELL_GREATER_MEND, 15}, {SPELL_LIFEBLOOM, 0} };
static const LearnEntry learn_garrick[] = { {SPELL_WARD, 3}, {SPELL_BREAKER, 8} };
static const LearnEntry learn_sera[]    = { {SPELL_EMBER, 1}, {SPELL_FROST, 4}, {SPELL_SPARK, 8}, {SPELL_GALE, 13} };
static const LearnEntry learn_finn[]    = { {SPELL_QUICKSTEP, 3}, {SPELL_DISRUPT, 7}, {SPELL_TREASURE_SENSE, 0} };
#define LEARN(a) a, (u8)(sizeof(a) / sizeof(a[0]))

const CharacterDef g_characters[CHR_COUNT] = {
    /*                 name       title                  hp  mp str def int agi luk  ghp gmp gstr gdef gint gagi gluk */
    [CHR_ROWAN]   = { "Rowan",   "the Courier",          30,  6,  8,  6,  4,  7,  6,  45, 12, 18, 14,  9,  16, 12, LEARN(learn_rowan),   SHEET_ROWAN, OPAL_ROWAN },
    [CHR_MIRA]    = { "Mira",    "the Wandering Healer", 22, 14,  4,  4,  9,  6,  7,  32, 30,  8, 10, 22,  13, 13, LEARN(learn_mira),    SHEET_TOWN,  OPAL_VILLAGER },
    [CHR_GARRICK] = { "Garrick", "the Exiled Guard",     40,  0, 10,  9,  2,  4,  4,  60,  5, 22, 21,  4,   9,  9, LEARN(learn_garrick), SHEET_TOWN,  OPAL_GUARD },
    [CHR_SERA]    = { "Sera",    "the Ruin Scholar",     20, 16,  3,  3, 11,  6,  6,  28, 34,  6,  8, 26,  12, 12, LEARN(learn_sera),    SHEET_TOWN,  OPAL_VILLAGER_B },
    [CHR_FINN]    = { "Finn",    "the Roadside Trickster",26, 6,  7,  5,  5, 11,  9,  38, 12, 16, 11, 10,  24, 18, LEARN(learn_finn),    SHEET_TOWN,  OPAL_CHILD },
};

/* ---------------------------------------------------------- items */
#define USE(n,d,p,e,pw) { n, d, p, ITYPE_USE, IFLAG_BATTLE | IFLAG_FIELD, e, pw, 0,0,0,0,0, EQ_ALL }
#define GEAR(n,d,p,t,a,df,i,ag,l,m) { n, d, p, t, 0, IEFF_NONE, 0, a, df, i, ag, l, m }
const ItemDef g_items[ITEM_COUNT] = {
    [ITEM_NONE]             = { "", "", 0, ITYPE_USE, 0, IEFF_NONE, 0, 0,0,0,0,0, 0 },
    [ITEM_HEALING_HERB]     = USE("Healing Herb",     "A bitter leaf. Restores 30 HP.",            8, IEFF_HEAL_HP, 30),
    [ITEM_SILVERLEAF_TONIC] = USE("Silverleaf Tonic", "A strong brew. Restores 90 HP.",           40, IEFF_HEAL_HP, 90),
    [ITEM_STARDEW_VIAL]     = USE("Stardew Vial",     "Dew gathered at dawn. Restores 20 MP.",    60, IEFF_HEAL_MP, 20),
    [ITEM_ANTIDOTE_ROOT]    = USE("Antidote Root",    "Chew to cure poison.",                     10, IEFF_CURE_POISON, 0),
    [ITEM_CLEARBELL]        = USE("Clearbell",        "Its ring cures sleep and silence.",        30, IEFF_CURE_ALL, 0),
    [ITEM_WAKEFEATHER]      = USE("Wakefeather",      "Revives a fallen ally with some HP.",     120, IEFF_REVIVE, 25),
    [ITEM_WAYSTONE]         = { "Waystone", "Returns you to a dungeon's entrance.", 25, ITYPE_USE, IFLAG_FIELD, IEFF_ESCAPE_DUNGEON, 0, 0,0,0,0,0, EQ_ALL },
    [ITEM_COURIER_KNIFE]    = GEAR("Courier Knife", "Light and quick. Atk +4.",          30, ITYPE_WEAPON,    4, 0, 0, 1, 0, EQ(CHR_ROWAN) | EQ(CHR_FINN)),
    [ITEM_OAK_STAFF]        = GEAR("Oak Staff",     "Steady in the hand. Atk +3 Int +2.", 40, ITYPE_WEAPON,   3, 0, 2, 0, 0, EQ(CHR_MIRA) | EQ(CHR_SERA)),
    [ITEM_IRON_SWORD]       = GEAR("Iron Sword",    "Plain and dependable. Atk +9.",    120, ITYPE_WEAPON,    9, 0, 0, 0, 0, EQ(CHR_ROWAN) | EQ(CHR_GARRICK)),
    [ITEM_HUNTING_BOW]      = GEAR("Hunting Bow",   "Strikes from the back row. Atk +7.", 110, ITYPE_WEAPON,  7, 0, 0, 2, 0, EQ(CHR_FINN)),
    [ITEM_PLANK_SHIELD]     = GEAR("Plank Shield",  "Nailed boards. Def +3.",            25, ITYPE_SHIELD,    0, 3, 0, 0, 0, EQ(CHR_ROWAN) | EQ(CHR_GARRICK)),
    [ITEM_LEATHER_CAP]      = GEAR("Leather Cap",   "Better than nothing. Def +2.",      20, ITYPE_HELM,      0, 2, 0, 0, 0, EQ_ALL),
    [ITEM_TRAVEL_COAT]      = GEAR("Travel Coat",   "Many pockets. Def +4 Agi +1.",      45, ITYPE_ARMOR,     0, 4, 0, 1, 0, EQ_ALL),
    [ITEM_LUCKY_BUTTON]     = GEAR("Lucky Button",  "A brass button. Luck +5.",          60, ITYPE_ACCESSORY, 0, 0, 0, 0, 5, EQ_ALL),
    [ITEM_STAR_SHARD]       = { "Star Shard", "A warm, pale fragment of a broken Star Seal.", 0, ITYPE_KEY, IFLAG_KEY, IEFF_NONE, 0, 0,0,0,0,0, 0 },
};

/* ---------------------------------------------------------- enemies (Milestone 2 slice) */
const EnemyDef g_enemies[ENEMY_COUNT] = {
    /*                        name              hp  mp str def int agi luk  exp gold  weak                    resist               drop                 rate ai */
    [ENEMY_MOSSFANG]       = { "Mossfang",      14,  0,  7,  3,  1,  6,  3,   4,  3, ELEM_BIT(ELEM_FIRE),     0,                   ITEM_HEALING_HERB,    48, AI_BASIC },
    [ENEMY_BRAMBLE_HOPPER] = { "Bramble Hopper",10,  0,  6,  2,  1, 11,  5,   3,  4, ELEM_BIT(ELEM_FIRE),     ELEM_BIT(ELEM_WIND), ITEM_HEALING_HERB,    32, AI_COWARD },
    [ENEMY_GLASSWING_WISP] = { "Glasswing Wisp",12,  8,  3,  2,  8,  9,  6,   6,  5, ELEM_BIT(ELEM_WIND),     ELEM_BIT(ELEM_LIGHT),ITEM_STARDEW_VIAL,    24, AI_CASTER },
    [ENEMY_MUDSHELL]       = { "Mudshell",      22,  0,  8,  9,  1,  2,  2,   7,  6, ELEM_BIT(ELEM_LIGHTNING),ELEM_BIT(ELEM_ICE),  ITEM_ANTIDOTE_ROOT,   40, AI_BASIC },
    [ENEMY_LANTERN_MIMIC]  = { "Lantern Mimic", 30, 10, 10,  6,  7,  5, 12,  20, 40, ELEM_BIT(ELEM_ICE),      ELEM_BIT(ELEM_FIRE), ITEM_LUCKY_BUTTON,    64, AI_CHARGER },
    [ENEMY_HOLLOW_SENTINEL]= { "Hollow Sentinel",80,12, 12, 10,  6,  4,  4,  45, 60, ELEM_BIT(ELEM_LIGHTNING),0,                   ITEM_SILVERLEAF_TONIC,255, AI_BOSS },
};

/* ---------------------------------------------------------- experience
 * Total experience needed to reach each level (index = level).
 * 12*(L-1)^2.15, tuned so story areas carry the party ~2-3 levels each. */
const u32 g_exp_table[MAX_LEVEL + 1] = {
    0, 0, 12, 53, 127, 236, 382, 565, 787, 1049,
    1351, 1695, 2081, 2509, 2980, 3494, 4053, 4656, 5305, 5998,
    6738, 7523, 8355, 9234, 10160, 11134, 12155, 13224, 14342, 15509,
    16724, 17988, 19302, 20666, 22079, 23543,
};
