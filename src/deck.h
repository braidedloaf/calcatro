#ifndef DECK_H
#define DECK_H

#include "game.h"

Deck create_deck(void);
void shuffle_deck(Deck *deck);
void init_hand(Hand *hand);
void draw_cards_to_hand(Hand *hand, int amount, Deck *deck, int *next_card);

#endif
