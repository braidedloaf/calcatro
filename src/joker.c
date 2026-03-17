#include "joker.h"

static int count_rank_matches(const Card *played_cards, int played_count, int matches_needed) {
    int counts[15] = { 0 };

    for (int i = 0; i < played_count; i++) {
        int rank = played_cards[i].rank;

        if (rank >= 0 && rank < 15) {
            counts[rank]++;
        }
    }

    for (int i = 0; i < 15; i++) {
        if (counts[i] >= matches_needed) {
            return 1;
        }
    }

    return 0;
}

void init_jokers(RunState *run_state) {
    run_state->joker_count = 0;

    for (int i = 0; i < MAX_JOKERS; i++) {
        run_state->jokers[i].type = JOKER_NONE;
        run_state->jokers[i].cost = 0;
        run_state->jokers[i].sell_value = 0;
        run_state->jokers[i].active = false;
    }
}

bool add_joker(RunState *run_state, JokerType type, int cost) {
    if (run_state->joker_count >= MAX_JOKERS) {
        return false;
    }

    for (int i = 0; i < MAX_JOKERS; i++) {
        if (!run_state->jokers[i].active) {
            run_state->jokers[i].type = type;
            run_state->jokers[i].cost = cost;
            run_state->jokers[i].sell_value = cost / 2;
            run_state->jokers[i].active = true;
            run_state->joker_count++;
            return true;
        }
    }

    return false;
}

void build_score_context(const EvaluatedHand *evaluated_hand, ScoreContext *context) {
    context->evaluated_hand = *evaluated_hand;
    context->chips = evaluated_hand->value.chips;
    context->mult = evaluated_hand->value.mult;
}

void apply_jokers(const RunState *run_state, const Card *played_cards, int played_count, ScoreContext *context) {
    for (int i = 0; i < run_state->joker_count && i < MAX_JOKERS; i++) {
        const Joker *joker = &run_state->jokers[i];

        if (!joker->active) {
            continue;
        }

        switch (joker->type) {
            case JOKER_BASIC_CHIPS:
                context->chips += 20;
                break;
            case JOKER_BASIC_MULT:
                context->mult += 4;
                break;
            case JOKER_PAIR_MULT:
                if (count_rank_matches(played_cards, played_count, 2)) {
                    context->mult += 8;
                }
                break;
            case JOKER_NONE:
            case JOKER_COUNT:
                break;
        }
    }
}

EvaluatedHand finalize_score_context(const ScoreContext *context) {
    EvaluatedHand result = context->evaluated_hand;

    result.value.chips = context->chips;
    result.value.mult = context->mult;
    return result;
}
