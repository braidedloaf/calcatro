#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef struct {
	char rank;
	char suit;
	int value;
} Card;
 
typedef struct {
	Card cards[52];
	int size;
} Deck;

typedef struct {
	int hand_size;
	int current_cards_cnt;
	Card hand[8];
} Hand;

int randomIntRange(int min, int max) {
	return min + (rand() % (max-min+1));
}

void shuffle_deck(Deck *deck) {
	for (int i = deck->size; i > 0; i--) {
		int j = rand() % (i+1);
		Card temp = deck->cards[i];
		deck->cards[i] = deck->cards[j];
		deck->cards[j] = temp;
	}
}

Deck create_deck() {
	Deck deck;
	for (int i = 0; i < 52; i++) {
		deck.cards[i].rank = (i % 13) < 8 ? '2' + (i % 13) : (i % 13) == 8 ? 'T' : (i % 13) == 9 ? 'J' : (i % 13) == 10 ? 'Q' : (i % 13) == 11 ? 'K' : 'A'; // J == 74 Q == 81 K == 75
		deck.cards[i].suit = (i / 13 == 0) ? 'S' : (i / 13 == 1) ? 'H' : (i / 13 == 2) ? 'C' : 'D';
		deck.cards[i].value = (i % 13) + 2 < 10 ? (i % 13) + 2 : (i % 13) + 2 == 14 ? 11: 10;
	}
	deck.size = 52;
	return deck;
}

void print_card(Card c) {
	gfx_PrintStringXY("Card: ", 10, 10);
    gfx_PrintChar(c.rank);
    gfx_PrintChar(c.suit);
	char s[3];
	sprintf(s, "%d", c.value);
	gfx_PrintString(s);
}

void draw_cards_to_hand(Hand *p_hand, int amt, Deck *p_deck, int *next_card) {	

	for (int i = 0; i < amt;) {
		if (*next_card > 52) {
			*next_card = 0;
			shuffle_deck(p_deck);
		}
		for (int j = 0; j < p_hand->hand_size; j++) {
			if (p_hand->hand[j] == NULL) {
				p_hand->hand[j] = p_deck->cards[(*next_card)++];
				i++;
				break;
			}
		}
	}

	return;
}



int main(void) {
	srand(time(NULL))

	kb_key_t key, prev_key = 0;
	bool running = true;

	gfx_Begin();
	gfx_SetTextScale(2,2);
	gfx_SetColor(0);

	Deck deck = create_deck();
	shuffle_deck(&deck);
	Hand hand;
	hand.hand_size = 8;
	hand.current_cards_cnt = 0;
	int next_card = 0;


	int card_idx = 0;
	while (running) {
		gfx_FillScreen(255);
		kb_Scan();
		
		key = kb_Data[7];


		if (key & kb_Left && !(prev_key & kb_Left)) {
			if (card_idx > 0)
				--card_idx;
		}
		if (key & kb_Right && !(prev_key & kb_Right)) {
			if (card_idx < deck.size-1) // 52 is deck size
				++card_idx;
		}

		prev_key = key;

		//print_card(deck.cards[card_idx]);
		
		if (hand.current_cards_cnt < hand.hand_size) {
			draw_cards_to_hand(&hand, hand.hand_size - hand.current_cards_cnt, &deck, &next_card);
		}

		if (kb_Data[6] & kb_Clear) { 
            running = false;
        }
		gfx_SwapDraw();
	}
	gfx_End();
	return 0;
}