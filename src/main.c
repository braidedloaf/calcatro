#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//screen is 320, 240


typedef struct {
	char rank;
	char suit;
	int value;
	bool is_selected;
	bool to_play;
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
	for (int i = deck->size - 1; i > 0; i--) {
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
		deck.cards[i].is_selected = false;
		deck.cards[i].to_play = false;
	}
	deck.size = 52;
	return deck;
}

void print_card(Card c , int x, int y) {
	gfx_SetTextXY(x, y);
	//gfx_PrintStringXY("Card: ", 10, 10);
    gfx_PrintChar(c.rank);
    gfx_PrintChar(c.suit);
	/*
	char s[3];
	sprintf(s, "%d", c.value);
	gfx_PrintString(s);
	*/
}

void display_hand(Hand *p_hand) {
	
	
	for (int i = 0; i < p_hand->current_cards_cnt; i++) {
		int offset_y = 0;
		if (p_hand->hand[i].is_selected || p_hand->hand[i].to_play)
			offset_y -= 20;
		print_card(p_hand->hand[i], 80 + (30 * i), offset_y + 200);
	}
	
}

void draw_cards_to_hand(Hand *p_hand, int amt, Deck *p_deck, int *next_card) {	

	for (int i = 0; i < amt;) {
		if (*next_card >= 52) {
			*next_card = 0;
			shuffle_deck(p_deck);
		}
		for (int j = 0; j < p_hand->hand_size; j++) {
			if (p_hand->hand[j].value == -1) {
				p_hand->hand[j] = p_deck->cards[(*next_card)++];
				i++;
				break;
			}
		}
	}
	p_hand->current_cards_cnt += amt;

	return;
}


int main(void) {
	srand(time(NULL));

	kb_key_t key, prev_key = 0;
	bool running = true;

	gfx_Begin();
	gfx_SetTextScale(1,2);
	gfx_SetColor(0);

	Deck deck = create_deck();
	shuffle_deck(&deck);
	
	Hand hand;
	memset(&hand, 0, sizeof(Hand)); // set all values to 0 bit
	hand.hand_size = 8;
	hand.current_cards_cnt = 0;
	for (int i = 0; i < hand.hand_size; i++) { //set cards value to be -1, to signify empty slot
		hand.hand[i].value = -1;
	}

	int next_card = 0;
	int card_idx = 0;
	
	while (running) {
		gfx_FillScreen(255);
		kb_Scan();
		
		
		if (hand.current_cards_cnt < hand.hand_size) {
			draw_cards_to_hand(&hand, hand.hand_size - hand.current_cards_cnt, &deck, &next_card);
		}
		
		hand.hand[card_idx].is_selected = true;
		key = kb_Data[1];
		if (key & kb_2nd) hand.hand[card_idx].to_play = !hand.hand[card_idx].to_play;

			
		//use arrow keys to change card to select
		key = kb_Data[7];
		if (key & kb_Left && !(prev_key & kb_Left)) {
			if (card_idx > 0) {
				hand.hand[card_idx--].is_selected = false;
			}
		}
		if (key & kb_Right && !(prev_key & kb_Right)) {
			if (card_idx < hand.hand_size-1) // 52 -> deck size | 8 -> hand_size
				hand.hand[card_idx++].is_selected = false;
		}

		prev_key = key;

		//print_card(hand.hand[card_idx], 10, 10);
		display_hand(&hand);

		if (kb_Data[6] & kb_Clear) { 
            running = false;
        }
		gfx_SwapDraw();
	}
	gfx_End();
	return 0;
}