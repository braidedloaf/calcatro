#include "deck.h"

#include <stdlib.h>
#include <string.h>

enum {
    DECK_SIZE = 52,
    HAND_SIZE = 8
};

void shuffle_deck(Deck *deck) {
    for (int i = deck->size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

Deck create_deck(void) {
    Deck deck;

    for (int i = 0; i < DECK_SIZE; i++) {
        deck.cards[i].rank = (i % 13) < 8 ? '2' + (i % 13) : (i % 13) == 8 ? 'T' : (i % 13) == 9 ? 'J' : (i % 13) == 10 ? 'Q' : (i % 13) == 11 ? 'K' : 'A';
        deck.cards[i].suit = (i / 13 == 0) ? 'H' : (i / 13 == 1) ? 'C' : (i / 13 == 2) ? 'D' : 'S';
        deck.cards[i].value = (i % 13) + 2 < 10 ? (i % 13) + 2 : (i % 13) + 2 == 14 ? 11 : 10;
        deck.cards[i].is_selected = false;
        deck.cards[i].to_play = false;
        deck.cards[i].sprite_row = i / 13;
        deck.cards[i].sprite_col = i % 13;
    }

    deck.size = DECK_SIZE;
    return deck;
}

void init_hand(Hand *hand) {
    memset(hand, 0, sizeof(*hand));
    hand->hand_size = HAND_SIZE;

    for (int i = 0; i < hand->hand_size; i++) {
        hand->hand[i].value = -1;
    }
}

void draw_cards_to_hand(Hand *hand, int amount, Deck *deck, int *next_card) {
    for (int drawn = 0; drawn < amount;) {
        if (*next_card >= DECK_SIZE) {
            *next_card = 0;
            shuffle_deck(deck);
        }

        for (int slot = 0; slot < hand->hand_size; slot++) {
            if (hand->hand[slot].value == -1) {
                hand->hand[slot] = deck->cards[(*next_card)++];
                drawn++;
                break;
            }
        }
    }

    hand->current_cards_cnt += amount;
}
