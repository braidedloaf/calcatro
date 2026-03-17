#include "gameplay.h"

#include "deck.h"
#include "hand_eval.h"
#include "joker.h"
#include "ui.h"

#include <graphx.h>

enum {
    INITIAL_HANDS = 4,
    INITIAL_DISCARDS = 3,
    INITIAL_MONEY = 4,
    INITIAL_ANTE = 0,
    INITIAL_BLIND = 0,
    MAX_PLAYING_CARDS = 5,
    MAX_SELECTED_CARDS = 5,
    SCORE_STEP_WAIT = 250,
    PRE_SCORE_WAIT = 500,
    POST_SCORE_WAIT = 500,
    SCORE_CLEAR_WAIT = 750
};

static void wait_frames(unsigned int frames) {
    for (unsigned int i = 0; i < frames; i++) {
        kb_Scan();
        gfx_Wait();
    }
}

static void init_game_round(GameRound *round) {
    round->arrow_prev_key = 0;
    round->select_prev_key = 0;
    round->discard_prev_key = 0;
    round->play_prev_key = 0;
    round->mode_prev_key = 0;
    round->next_card = 0;
    round->card_idx = 0;
    round->held_card_idx = -1;
    round->mode = UI_MODE_NORMAL;
    round->deck = create_deck();
    shuffle_deck(&round->deck);
    init_hand(&round->hand);
}

static void build_playing_hand(const Hand *hand, Card *playing_hand, int *playing_hand_count) {
    *playing_hand_count = 0;
    for (int i = 0; i < hand->hand_size; i++) {
        if (hand->hand[i].to_play) {
            playing_hand[(*playing_hand_count)++] = hand->hand[i];
        }
    }
}

static void toggle_current_card(GameRound *round, kb_key_t select_key) {
    round->hand.hand[round->card_idx].is_selected = true;

    if ((select_key & kb_2nd) && !(round->select_prev_key & kb_2nd)) {
        if (!round->hand.hand[round->card_idx].to_play && round->hand.amt_selected < MAX_SELECTED_CARDS) {
            round->hand.hand[round->card_idx].to_play = true;
            round->hand.amt_selected++;
        } else if (round->hand.hand[round->card_idx].to_play) {
            round->hand.hand[round->card_idx].to_play = false;
            if (round->hand.amt_selected > 0) {
                round->hand.amt_selected--;
            }
        }
    }

    round->select_prev_key = select_key;
}

static void swap_hand_cards(Hand *hand, int left, int right) {
    Card temp = hand->hand[left];
    hand->hand[left] = hand->hand[right];
    hand->hand[right] = temp;
}

static void discard_selected_cards(GameRound *round, RunState *run_state) {
    for (int i = 0; i < round->hand.hand_size; i++) {
        if (round->hand.hand[i].to_play) {
            round->hand.hand[i].value = -1;
            round->hand.hand[i].to_play = false;
            round->hand.hand[i].is_selected = false;
            round->hand.current_cards_cnt--;
        }
    }

    round->hand.amt_selected = 0;
    run_state->discards_left--;
}

static int clear_played_cards(GameRound *round) {
    int played_count = 0;

    for (int i = 0; i < round->hand.hand_size; i++) {
        if (round->hand.hand[i].to_play) {
            round->hand.hand[i].value = -1;
            round->hand.hand[i].to_play = false;
            round->hand.hand[i].is_selected = false;
            played_count++;
        }
    }

    round->hand.amt_selected = 0;
    return played_count;
}

static int animate_scoring(const Card *playing_hand, int playing_hand_count, const EvaluatedHand *result, Hand *hand, const RunState *run_state) {
    int running_total = result->value.chips;

    for (int i = 0; i < result->scoring_count; i++) {
        HandValue display = result->value;

        running_total += result->scoring_cards[i].value;
        display.chips = running_total;

        gfx_FillScreen(255);
        display_hand(hand);
        display_playing_hand(playing_hand, playing_hand_count, result->scoring_cards, i);
        display_game_stats(run_state->score, run_state->target_score, 0, run_state->hands_left, run_state->discards_left, run_state->money, display);
        gfx_SwapDraw();
        wait_frames(SCORE_STEP_WAIT);
    }

    return running_total * result->value.mult;
}

static void resolve_played_hand(GameRound *round, RunState *run_state, const Card *playing_hand, int playing_hand_count, const EvaluatedHand *result) {
    int played_count = clear_played_cards(round);

    run_state->hands_left--;

    gfx_FillScreen(255);
    display_hand(&round->hand);
    display_playing_hand(playing_hand, playing_hand_count, result->scoring_cards, -1);
    display_game_stats(run_state->score, run_state->target_score, 0, run_state->hands_left, run_state->discards_left, run_state->money, result->value);
    gfx_SwapDraw();
    wait_frames(PRE_SCORE_WAIT);

    int gained = animate_scoring(playing_hand, playing_hand_count, result, &round->hand, run_state);
    HandValue post_score_value = result->value;

    post_score_value.chips = gained / result->value.mult;
    post_score_value.mult = result->value.mult;

    gfx_FillScreen(255);
    display_hand(&round->hand);
    display_playing_hand(playing_hand, playing_hand_count, result->scoring_cards, -1);
    display_game_stats(run_state->score, run_state->target_score, gained, run_state->hands_left, run_state->discards_left, run_state->money, post_score_value);
    gfx_SwapDraw();
    wait_frames(POST_SCORE_WAIT);

    gfx_FillScreen(255);
    display_hand(&round->hand);
    display_playing_hand(playing_hand, playing_hand_count, result->scoring_cards, -1);
    display_game_stats(run_state->score, run_state->target_score, gained, run_state->hands_left, run_state->discards_left, run_state->money, (HandValue){-1, -1, 0});
    gfx_SwapDraw();
    wait_frames(SCORE_CLEAR_WAIT);

    run_state->score += gained;
    round->hand.current_cards_cnt -= played_count;
}

static void move_card_selection(GameRound *round, kb_key_t arrow_key) {
    if ((arrow_key & kb_Left) && !(round->arrow_prev_key & kb_Left)) {
        if (round->card_idx == 0) {
            round->hand.hand[round->card_idx].is_selected = false;
            round->card_idx = round->hand.hand_size - 1;
        } else {
            round->hand.hand[round->card_idx--].is_selected = false;
        }
    }

    if ((arrow_key & kb_Right) && !(round->arrow_prev_key & kb_Right)) {
        if (round->card_idx == round->hand.hand_size - 1) {
            round->hand.hand[round->card_idx].is_selected = false;
            round->card_idx = 0;
        } else {
            round->hand.hand[round->card_idx++].is_selected = false;
        }
    }

    round->arrow_prev_key = arrow_key;
}

static void move_held_card(GameRound *round, kb_key_t arrow_key) {
    if ((arrow_key & kb_Left) && !(round->arrow_prev_key & kb_Left) && round->held_card_idx > 0) {
        swap_hand_cards(&round->hand, round->held_card_idx, round->held_card_idx - 1);
        round->held_card_idx--;
        round->card_idx = round->held_card_idx;
    }

    if ((arrow_key & kb_Right) && !(round->arrow_prev_key & kb_Right) && round->held_card_idx < round->hand.hand_size - 1) {
        swap_hand_cards(&round->hand, round->held_card_idx, round->held_card_idx + 1);
        round->held_card_idx++;
        round->card_idx = round->held_card_idx;
    }

    round->arrow_prev_key = arrow_key;
}

static void toggle_reorder_mode(GameRound *round) {
    if (round->mode == UI_MODE_REORDER) {
        round->mode = UI_MODE_NORMAL;
        round->held_card_idx = -1;
        return;
    }

    round->mode = UI_MODE_REORDER;
    round->held_card_idx = round->card_idx;
}

static void handle_reorder_input(GameRound *round, kb_key_t mode_key, kb_key_t arrow_key) {
    if (round->held_card_idx < 0) {
        move_card_selection(round, arrow_key);
    } else {
        move_held_card(round, arrow_key);
    }

    if ((mode_key & kb_Mode) && !(round->mode_prev_key & kb_Mode)) {
        if (round->held_card_idx < 0) {
            round->held_card_idx = round->card_idx;
        } else {
            round->held_card_idx = -1;
            round->mode = UI_MODE_NORMAL;
        }
    }
}

void init_run_state(RunState *run_state) {
    run_state->score = 0;
    run_state->target_score = 0;
    run_state->hands_left = INITIAL_HANDS;
    run_state->discards_left = INITIAL_DISCARDS;
    run_state->money = INITIAL_MONEY;
    run_state->current_ante = INITIAL_ANTE;
    run_state->current_blind = INITIAL_BLIND;
    init_jokers(run_state);
}

GameState handle_game_state(RunState *run_state, bool *quit, unsigned int *frame_timer) {
    GameRound round;

    gfx_SetTextScale(1, 2);
    gfx_SetColor(0);
    init_game_round(&round);

    while (true) {
        kb_key_t select_key;
        kb_key_t discard_key;
        kb_key_t play_key;
        kb_key_t mode_key;
        kb_key_t arrow_key;
        Card playing_hand[MAX_PLAYING_CARDS];
        int playing_hand_count;
        ScoreContext score_context;
        EvaluatedHand result;

        gfx_FillScreen(255);
        (*frame_timer)++;
        kb_Scan();

        if (round.hand.current_cards_cnt < round.hand.hand_size) {
            draw_cards_to_hand(&round.hand, round.hand.hand_size - round.hand.current_cards_cnt, &round.deck, &round.next_card);
        }

        select_key = kb_Data[1];
        arrow_key = kb_Data[7];
        mode_key = kb_Data[1];

        if (round.mode == UI_MODE_REORDER) {
            handle_reorder_input(&round, mode_key, arrow_key);
        } else {
            move_card_selection(&round, arrow_key);
            toggle_current_card(&round, select_key);

            if ((mode_key & kb_Mode) && !(round.mode_prev_key & kb_Mode)) {
                toggle_reorder_mode(&round);
            }
        }
        build_playing_hand(&round.hand, playing_hand, &playing_hand_count);
        result = get_hand_type(playing_hand, playing_hand_count);
        build_score_context(&result, &score_context);
        apply_jokers(run_state, playing_hand, playing_hand_count, &score_context);
        result = finalize_score_context(&score_context);

        discard_key = kb_Data[3];
        if ((discard_key & kb_GraphVar) && !(round.discard_prev_key & kb_GraphVar) && run_state->discards_left > 0 && round.hand.amt_selected > 0 && round.mode == UI_MODE_NORMAL) {
            discard_selected_cards(&round, run_state);
        }
        round.discard_prev_key = discard_key;

        play_key = kb_Data[2];
        if ((play_key & kb_Alpha) && !(round.play_prev_key & kb_Alpha) && run_state->hands_left > 0 && round.hand.amt_selected > 0 && round.mode == UI_MODE_NORMAL) {
            resolve_played_hand(&round, run_state, playing_hand, playing_hand_count, &result);
        }
        round.play_prev_key = play_key;
        round.mode_prev_key = mode_key;

        display_joker_row(run_state);
        display_hand_ui(&round.hand, UI_FOCUS_HAND, round.card_idx, round.held_card_idx, round.mode);
        draw_gameplay_footer(UI_FOCUS_HAND, round.mode, round.held_card_idx);
        display_game_stats(run_state->score, run_state->target_score, 0, run_state->hands_left, run_state->discards_left, run_state->money, result.value);

        if (run_state->score >= run_state->target_score) {
            run_state->score = 0;
            if (run_state->current_blind == 2) {
                run_state->current_blind = 0;
                run_state->current_ante++;
            } else {
                run_state->current_blind++;
            }
            return STATE_SHOP;
        }

        if (kb_Data[6] & kb_Clear) {
            *quit = true;
            return STATE_GAME;
        }

        gfx_SwapDraw();
    }
}
