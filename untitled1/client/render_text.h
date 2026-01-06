#ifndef RENDER_TEXT_H
#define RENDER_TEXT_H

#include "../common/ipc.h"

// inicializacia (pre textový render vlastne nič netreba)
void render_init_text();

// ukončenie renderu
void render_cleanup_text();

// vykreslenie herného sveta do konzoly
void render_game_text(SharedGame *game, int id);

#endif