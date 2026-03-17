#ifndef JOKER_H
#define JOKER_H

#include "game.h"

typedef struct {
    EvaluatedHand evaluated_hand;
    int chips;
    int mult;
} ScoreContext;

void init_jokers(RunState *run_state);
bool add_joker(RunState *run_state, JokerType type, int cost);
void build_score_context(const EvaluatedHand *evaluated_hand, ScoreContext *context);
void apply_jokers(const RunState *run_state, const Card *played_cards, int played_count, ScoreContext *context);
EvaluatedHand finalize_score_context(const ScoreContext *context);

#endif
