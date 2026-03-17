#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_RULES,
    STATE_BLIND_SELECT,
    STATE_SHOP
} GameState;

typedef struct {
    char rank;
    char suit;
    int value;
    bool is_selected;
    bool to_play;
    int sprite_row;
    int sprite_col;
} Card;

typedef struct {
    Card cards[52];
    int size;
} Deck;

typedef struct {
    int hand_size;
    int current_cards_cnt;
    int amt_selected;
    Card hand[8];
} Hand;

typedef enum {
    HAND_HIGH_CARD = 0,
    HAND_ONE_PAIR,
    HAND_TWO_PAIR,
    HAND_THREE_KIND,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULL_HOUSE,
    HAND_FOUR_KIND,
    HAND_STRAIGHT_FLUSH,
    HAND_ROYAL_FLUSH,
    HAND_FIVE_KIND,
    HAND_FLUSH_HOUSE,
    HAND_FLUSH_FIVE,
    HAND_COUNT
} HandType;

typedef struct {
    HandType type;
    int chips;
    int mult;
} HandValue;

typedef struct {
    HandValue value;
    Card scoring_cards[5];
    int scoring_count;
} EvaluatedHand;

typedef struct {
    int bonus_chips;
    int bonus_mult;
} HandUpgrade;

typedef struct {
    int score;
    int target_score;
    int hands_left;
    int discards_left;
    int money;
    int current_ante;
    int current_blind;
} RunState;

#endif
