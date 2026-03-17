#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "game.h"
#include "ui.h"

#include <keypadc.h>

typedef struct {
    kb_key_t arrow_prev_key;
    kb_key_t select_prev_key;
    kb_key_t discard_prev_key;
    kb_key_t play_prev_key;
    kb_key_t mode_prev_key;
    int next_card;
    int card_idx;
    int held_card_idx;
    UiMode mode;
    Deck deck;
    Hand hand;
} GameRound;

void init_run_state(RunState *run_state);
GameState handle_game_state(RunState *run_state, bool *quit, unsigned int *frame_timer);

#endif
