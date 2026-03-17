#include "hand_eval.h"

#include <string.h>

HandValue hand_table[HAND_COUNT] = {
    { HAND_HIGH_CARD,      5,   1 },
    { HAND_ONE_PAIR,       10,  2 },
    { HAND_TWO_PAIR,       20,  2 },
    { HAND_THREE_KIND,     30,  3 },
    { HAND_STRAIGHT,       30,  4 },
    { HAND_FLUSH,          35,  4 },
    { HAND_FULL_HOUSE,     40,  4 },
    { HAND_FOUR_KIND,      60,  7 },
    { HAND_STRAIGHT_FLUSH, 100, 8 },
    { HAND_ROYAL_FLUSH,    100, 8 },
    { HAND_FIVE_KIND,      120, 12 },
    { HAND_FLUSH_HOUSE,    140, 14 },
    { HAND_FLUSH_FIVE,     160, 16 }
};

const HandUpgrade upgrade_table[HAND_COUNT] = {
    [HAND_HIGH_CARD]      = {10, 1},
    [HAND_ONE_PAIR]       = {15, 1},
    [HAND_TWO_PAIR]       = {20, 1},
    [HAND_THREE_KIND]     = {20, 2},
    [HAND_STRAIGHT]       = {30, 3},
    [HAND_FLUSH]          = {15, 2},
    [HAND_FULL_HOUSE]     = {25, 2},
    [HAND_FOUR_KIND]      = {30, 3},
    [HAND_STRAIGHT_FLUSH] = {40, 4},
    [HAND_ROYAL_FLUSH]    = {0, 0},
    [HAND_FIVE_KIND]      = {0, 0},
    [HAND_FLUSH_HOUSE]    = {0, 0},
    [HAND_FLUSH_FIVE]     = {0, 0}
};

const HandType shape_hand_map[9] = {
    HAND_HIGH_CARD,
    HAND_ONE_PAIR,
    HAND_TWO_PAIR,
    HAND_THREE_KIND,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULL_HOUSE,
    HAND_FOUR_KIND,
    HAND_STRAIGHT_FLUSH
};

int card_equal(const Card *left, const Card *right) {
    return left->rank == right->rank &&
           left->value == right->value &&
           left->suit == right->suit;
}

EvaluatedHand get_hand_type(Card *p_cards, int count) {
    Card cards[count];
    memcpy(cards, p_cards, count * sizeof(Card));

    EvaluatedHand result;
    result.scoring_count = 0;

    if (count == 0) {
        result.value = (HandValue){ -1, 0, 0 };
        return result;
    }

    if (count < 2 || count > 5) {
        result.value = hand_table[HAND_HIGH_CARD];
        result.scoring_count = count;
        memcpy(result.scoring_cards, p_cards, sizeof(Card) * count);
        return result;
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (cards[i].rank > cards[j].rank) {
                Card tmp = cards[i];
                cards[i] = cards[j];
                cards[j] = tmp;
            }
        }
    }

    int rank_count[128] = {0};
    int suit_count[4] = {0};

    for (int i = 0; i < count; i++) {
        rank_count[(int)cards[i].rank]++;
        switch (cards[i].suit) {
            case 'S': suit_count[0]++; break;
            case 'H': suit_count[1]++; break;
            case 'C': suit_count[2]++; break;
            case 'D': suit_count[3]++; break;
        }
    }

    bool is_flush = false;
    char flush_suit = 0;
    for (int i = 0; i < 4; i++) {
        if (suit_count[i] == 5) {
            is_flush = true;
            flush_suit = "SHCD"[i];
            break;
        }
    }

    bool is_straight = false;
    int straight_high_rank = 0;
    const char straight_ranks[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};
    for (int i = 0; i <= 8; i++) {
        bool all_in_a_row = true;
        for (int j = 0; j < 5; j++) {
            if (rank_count[(int)straight_ranks[i + j]] == 0) {
                all_in_a_row = false;
                break;
            }
        }
        if (all_in_a_row) {
            is_straight = true;
            straight_high_rank = (int)straight_ranks[i + 4];
            break;
        }
    }

    if (!is_straight && count == 5 &&
        rank_count['A'] && rank_count['2'] &&
        rank_count['3'] && rank_count['4'] &&
        rank_count['5']) {
        is_straight = true;
        straight_high_rank = '5';
    }

    int pairs = 0;
    int trips = 0;
    int quads = 0;
    int quints = 0;
    char pair_ranks[2] = {0};
    char trip_rank = 0;
    char quad_rank = 0;
    char quint_rank = 0;

    for (const char *rank = "23456789TJQKA"; *rank; rank++) {
        int n = rank_count[(int)*rank];
        if (n == 5) {
            quints++;
            quint_rank = *rank;
        } else if (n == 4) {
            quads++;
            quad_rank = *rank;
        } else if (n == 3) {
            trips++;
            trip_rank = *rank;
        } else if (n == 2) {
            if (pairs < 2) {
                pair_ranks[pairs] = *rank;
            }
            pairs++;
        }
    }

#define MATCH_RANKS(ranklist, num) \
    for (int i = 0; i < count; i++) { \
        for (int rank_index = 0; rank_index < (num); rank_index++) { \
            if (p_cards[i].rank == (ranklist)[rank_index]) { \
                result.scoring_cards[result.scoring_count++] = p_cards[i]; \
                break; \
            } \
        } \
    }

#define MATCH_ALL() \
    for (int i = 0; i < count; i++) { \
        result.scoring_cards[result.scoring_count++] = p_cards[i]; \
    }

#define MATCH_FLUSH(flush) \
    for (int i = 0; i < count; i++) { \
        if (p_cards[i].suit == (flush)) { \
            result.scoring_cards[result.scoring_count++] = p_cards[i]; \
        } \
    }

    if (quints && is_flush) {
        result.value = hand_table[HAND_FLUSH_FIVE];
        MATCH_RANKS(&quint_rank, 1);
    } else if (trips && pairs && is_flush) {
        char ranks[2] = { trip_rank, pair_ranks[0] };
        result.value = hand_table[HAND_FLUSH_HOUSE];
        MATCH_RANKS(ranks, 2);
    } else if (quints) {
        result.value = hand_table[HAND_FIVE_KIND];
        MATCH_RANKS(&quint_rank, 1);
    } else if (count == 5 && is_straight && is_flush && straight_high_rank == 'A') {
        result.value = hand_table[HAND_ROYAL_FLUSH];
        MATCH_ALL();
    } else if (count == 5 && is_straight && is_flush) {
        result.value = hand_table[HAND_STRAIGHT_FLUSH];
        MATCH_ALL();
    } else if (quads) {
        result.value = hand_table[HAND_FOUR_KIND];
        MATCH_RANKS(&quad_rank, 1);
    } else if (trips && pairs) {
        char ranks[2] = { trip_rank, pair_ranks[0] };
        result.value = hand_table[HAND_FULL_HOUSE];
        MATCH_RANKS(ranks, 2);
    } else if (is_flush) {
        result.value = hand_table[HAND_FLUSH];
        MATCH_FLUSH(flush_suit);
    } else if (is_straight) {
        result.value = hand_table[HAND_STRAIGHT];
        MATCH_ALL();
    } else if (trips) {
        result.value = hand_table[HAND_THREE_KIND];
        MATCH_RANKS(&trip_rank, 1);
    } else if (pairs >= 2) {
        result.value = hand_table[HAND_TWO_PAIR];
        MATCH_RANKS(pair_ranks, 2);
    } else if (pairs == 1) {
        result.value = hand_table[HAND_ONE_PAIR];
        MATCH_RANKS(&pair_ranks[0], 1);
    } else {
        Card highest = cards[count - 1];

        result.value = hand_table[HAND_HIGH_CARD];
        for (int i = 0; i < count; i++) {
            if (card_equal(&p_cards[i], &highest)) {
                result.scoring_cards[0] = p_cards[i];
                result.scoring_count = 1;
                break;
            }
        }
    }

#undef MATCH_RANKS
#undef MATCH_ALL
#undef MATCH_FLUSH

    return result;
}
