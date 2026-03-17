#ifndef HAND_EVAL_H
#define HAND_EVAL_H

#include "game.h"

extern HandValue hand_table[HAND_COUNT];
extern const HandUpgrade upgrade_table[HAND_COUNT];
extern const HandType shape_hand_map[9];

int card_equal(const Card *left, const Card *right);
EvaluatedHand get_hand_type(Card *cards, int count);

#endif
