/* data_maps.c - maps, NPCs, events, warps and Chapter 1 story text */
#include "data.h"
#include "assets.h"

#define S(pages) { pages, (u8)(sizeof(pages) / sizeof(pages[0])) }
#define ALWAYS FLAG_NONE

/* ================================================================ prologue */
const char * const g_intro_pages[] = {
    "Three nights ago,\nthe great beacon of Crownfall\nwent dark.",
    "Across Asterra, the seven\nStar Seals began to crack.\nRivers wandered.\nForests fell silent.",
    "The royal court blamed\nan enemy across the sea.\nFew in the borderlands\nbelieved it.",
    "Last night, monsters came\nto Brindleford.",
    "By dawn the old mill lay\nin ruins, and the Elder\nhad sent for the village\ncourier.",
    0
};

const char * const g_dream_pages[] = {
    "That night, a pale light\nfills your dreams.",
    "It speaks without a voice:\n\n\"Not invaders.\nNot ever.\"",
    "END OF MILESTONE 1\n\nThanks for playing!\nThe road north opens\nin the next build.",
    0
};

/* ================================================================ Brindleford */
static const Page p_tamsin[] = {
    { ALWAYS, 0, ACT_NONE,
      "Tamsin: Road north's closed, $. Elder's orders.\f"
      "Nobody leaves until we know what crawled out of the Thornwall." },
    { FLAG_FOUND_SHARD, 0, ACT_NONE,
      "Tamsin: Is your pocket... glowing?\f"
      "Right. I'm not paid enough for glowing. Road's still closed." },
};
static const Page p_hesta[] = {
    { ALWAYS, FLAG_TALKED_ELDER, ACT_NONE,
      "Hesta: $, thank the stars you're whole.\f"
      "Those beasts went straight for the old mill last night.\f"
      "Not the barns. Not the houses. The mill.\f"
      "Something there called to them. Search the rubble by the east fence for me?\f"
      "You've the sharpest eyes in Brindleford." },
    { FLAG_TALKED_ELDER, 0, ACT_NONE,
      "Hesta: The mill's by the east fence, past the well. Mind the loose beams." },
    { FLAG_FOUND_SHARD, 0, ACT_NONE,
      "Hesta: ...A piece of a Star Seal. Here, in my village.\f"
      "The capital swears invaders broke the Seals.\f"
      "But Seals don't crack from the outside, child.\f"
      "Rest at the inn tonight. Tomorrow we decide what that light wants with you." },
};
static const Page p_pip[] = {
    { ALWAYS, 0, ACT_NONE,
      "Pip: Did you SEE them? Eyes like lanterns! Mum says I dreamed it.\f"
      "I didn't dream it." },
    { FLAG_FOUND_SHARD, 0, ACT_NONE,
      "Pip: Your pocket's shining! Can I hold it?\f...No? Fine. I'll find my own." },
};
static const Page p_odo[] = {
    { ALWAYS, 0, ACT_NONE,
      "Odo: Lost two fences and half my turnips.\f"
      "Fences I can mend. Those turnips were my best work, $." },
};
static const Page p_nell[] = {
    { ALWAYS, 0, ACT_NONE,
      "Nell: The shops are shut while we patch the roofs.\f"
      "The merchants will be back once the mill's cleared." },
};
static const NpcDef npcs_brindleford[] = {
    { 14,  1, SHEET_TOWN, OPAL_GUARD,      DIR_DOWN, MOVE_STATIC, S(p_tamsin) },
    { 16,  7, SHEET_TOWN, OPAL_ELDER,      DIR_LEFT, MOVE_STATIC, S(p_hesta) },
    {  9,  8, SHEET_TOWN, OPAL_CHILD,      DIR_DOWN, MOVE_WANDER, S(p_pip) },
    { 19, 14, SHEET_TOWN, OPAL_VILLAGER,   DIR_RIGHT, MOVE_STATIC, S(p_odo) },
    {  8, 12, SHEET_TOWN, OPAL_VILLAGER_B, DIR_DOWN, MOVE_WANDER, S(p_nell) },
};

static const Page e_inn_sign[]   = { { ALWAYS, 0, ACT_NONE, "THE KETTLE & KEY\nBeds, broth, and gossip." } };
static const Page e_shop_sign[]  = { { ALWAYS, 0, ACT_NONE, "WICKHAM'S GOODS\nA note pinned below: Closed for repairs." } };
static const Page e_south_sign[] = { { ALWAYS, 0, ACT_NONE,
    "SOUTH: The Low Fields.\fSomeone has scratched underneath: STAY INSIDE AFTER DARK." } };
static const Page e_home[]       = { { ALWAYS, 0, ACT_NONE,
    "Home. Your courier bag hangs by the door, empty for once." } };
static const Page e_shop_door[]  = { { ALWAYS, 0, ACT_NONE, "Locked. Hammering echoes from inside." } };
static const Page e_elder_door[] = { { ALWAYS, 0, ACT_NONE, "Elder Hesta's house. She's out by the well." } };
static const Page e_well[]       = { { ALWAYS, 0, ACT_NONE,
    "The village well. The water is cold enough to wake the dead.\fNot literally, you hope." } };
static const Page e_mill[] = {
    { ALWAYS, 0, ACT_NONE, "Charred beams and split stone. This was the old mill." },
    { FLAG_TALKED_ELDER, FLAG_FOUND_SHARD, ACT_GET_STAR_SHARD,
      "Something glints beneath a fallen beam...\f"
      TEXT_SFX_ITEM "$ found a Star Shard!\f"
      "The pale stone is warm. Its light pulses, slow and steady, like a heartbeat." },
    { FLAG_FOUND_SHARD, 0, ACT_NONE, "Only ash and broken stone remain." },
};
static const EventDef events_brindleford[] = {
    {  9,  4, S(e_inn_sign) },
    {  3, 12, S(e_shop_sign) },
    { 13, 17, S(e_south_sign) },
    { 23,  4, S(e_home) },
    {  4, 11, S(e_shop_door) },
    { 22, 11, S(e_elder_door) },
    { 14,  7, S(e_well) },
    { 23, 16, S(e_mill) },
};
static const WarpDef warps_brindleford[] = {
    { 7, 4, MAP_INN, 7, 8, DIR_UP },
};

/* ================================================================ Kettle & Key inn */
static const Page p_bram[] = {
    { ALWAYS, 0, ACT_INN,
      "Bram: Welcome to the Kettle & Key!\f"
      "After last night, the first bed's free for anyone who helped.\f"
      "Stay the night?" },
};
static const Page p_traveler[] = {
    { ALWAYS, 0, ACT_NONE,
      "Traveler: I came to Brindleford for the peace and quiet.\f"
      "Excellent timing on my part." },
};
static const NpcDef npcs_inn[] = {
    {  3, 2, SHEET_TOWN, OPAL_INNKEEP,  DIR_DOWN, MOVE_STATIC, S(p_bram) },
    { 11, 6, SHEET_TOWN, OPAL_VILLAGER, DIR_LEFT, MOVE_WANDER, S(p_traveler) },
};
static const WarpDef warps_inn[] = {
    { 7, 9, MAP_BRINDLEFORD, 7, 5, DIR_DOWN },
};

/* ================================================================ map table */
#define LIST(a) a, (u8)(sizeof(a) / sizeof(a[0]))
const MapDef g_maps[MAP_COUNT] = {
    [MAP_BRINDLEFORD] = { "Brindleford", MAPW_BRINDLEFORD, MAPH_BRINDLEFORD, maptiles_brindleford,
                          LIST(npcs_brindleford), LIST(events_brindleford), LIST(warps_brindleford) },
    [MAP_INN]         = { "Kettle & Key", MAPW_INN, MAPH_INN, maptiles_inn,
                          LIST(npcs_inn), 0, 0, LIST(warps_inn) },
};
