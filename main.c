/* main.c - The Shattered Crown: boot, title, new game, field loop */
#include "game.h"
#include "assets.h"

int main(void)
{
    video_init();
    settings_default();
    audio_init();

    title_run();

    /* New Game */
    fade_out();
    game_new();
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;     /* text only for the prologue */
    oam_hide_all();
    narration(g_intro_pages, 1);

    field_enter(MAP_BRINDLEFORD, 23, 5, DIR_DOWN);
    music_play(MUS_TOWN);
    fade_in();
    dialog_run("Morning light over Brindleford. Smoke still rises from the east side.\f"
               "Elder Hesta asked for you. She'll be by the well.", 0);
    field_run();
    return 0;
}
