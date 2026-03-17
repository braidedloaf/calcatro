#ifndef UI_H
#define UI_H

#include "game.h"

typedef enum {
    UI_FOCUS_HAND,
    UI_FOCUS_ACTIONS
} UiFocusRegion;

typedef enum {
    UI_MODE_NORMAL,
    UI_MODE_REORDER,
    UI_MODE_SELL
} UiMode;

typedef enum {
    ACTION_PLAY,
    ACTION_DISCARD,
    ACTION_REORDER,
    ACTION_COUNT
} GameplayAction;

void draw_main_menu(void);
void draw_rules_menu(void);
void draw_blind_menu(int score, int hands_left, int discards_left, int money, HandValue hand_value, int current_blind, int focused_blind, int current_ante, const int *base_ante_values);
void draw_shop(int score, int hands_left, int discards_left, int money, HandValue hand_value, int planet_index_1, int planet_index_2, const bool *bought, int focused_slot);
void draw_pre_shop(int score, int hands_left, int discards_left, int money, HandValue hand_value, int current_blind, bool focused_continue);
void display_hand(const Hand *hand);
void display_hand_ui(const Hand *hand, UiFocusRegion focus_region, int focused_index, int held_index, UiMode mode);
void display_playing_hand(const Card *cards, int count, const Card *scoring_cards, int pos);
void display_game_stats(int score, int target_score, int hand_score, int hands_left, int discards_left, int money, HandValue hand_value);
void draw_gameplay_actions(UiFocusRegion focus_region, int focused_action, UiMode mode);
void draw_gameplay_footer(UiFocusRegion focus_region, UiMode mode, int held_index);
void draw_blind_footer(void);
void draw_shop_footer(bool in_pre_shop);

#endif
