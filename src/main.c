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
	int amt_selected;
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
	char left = '>';
    char right = '<';

    if (c.is_selected) {
        left = '<';
        right = '>';
    }

    if (c.is_selected || c.to_play) {
        gfx_SetTextXY(x - 8, y);
        gfx_PrintChar(left);
    }

    gfx_SetTextXY(x, y);
    gfx_PrintChar(c.rank);
    gfx_PrintChar(c.suit);

    if (c.is_selected || c.to_play) {
        gfx_SetTextXY(x + 16, y);
        gfx_PrintChar(right);
    }

}

void display_game_stats(int score, int target_score, int hands_left, int discards_left) {
	gfx_SetTextXY(10, 20);
	gfx_PrintInt(target_score, 1);
	gfx_SetTextXY(10, 36);
	gfx_PrintInt(score, 1);
	gfx_SetTextXY(10, 52);
	gfx_PrintString("H: ");
	gfx_PrintInt(hands_left, 1);
	gfx_PrintString(" D: ");
	gfx_PrintInt(discards_left, 1);
}

void display_hand(Hand *p_hand) {
	
	for (int i = 0; i < p_hand->current_cards_cnt; i++) {
		int offset_y = 0;
		int offset_x = 74;
		if (p_hand->hand[i].is_selected || p_hand->hand[i].to_play) {
			offset_y -= 16;
		}
			
		print_card(p_hand->hand[i], offset_x + (32 * i), offset_y + 200);
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

	kb_key_t arrow_key, arrow_prev_key = 0, select_key, select_prev_key = 0, discard_key, discard_prev_key = 0;
	bool running = true;

	int score = 0, target_score = 0, hands_left = 4, discards_left = 3;

	gfx_Begin();
	gfx_SetTextScale(1,2);
	gfx_SetColor(0);

	Deck deck = create_deck();
	shuffle_deck(&deck);
	
	Hand hand;
	memset(&hand, 0, sizeof(Hand)); // set all values to 0 bit
	hand.hand_size = 8;
	hand.amt_selected = 0;
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
		
		select_key = kb_Data[1];
		hand.hand[card_idx].is_selected = true;
		if ((select_key & kb_2nd) && !(select_prev_key & kb_2nd)) {
			if (!hand.hand[card_idx].to_play && hand.amt_selected < 5) {
				// If not selected yet, and we have room, allow selection
				hand.hand[card_idx].to_play = true;
				hand.amt_selected++;
			} else if (hand.hand[card_idx].to_play) {
				// Always allow deselection
				hand.hand[card_idx].to_play = false;
				if (hand.amt_selected > 0)
					hand.amt_selected--;
			}
		}
		select_prev_key = select_key;

		discard_key = kb_Data[3];
		if (discard_key & kb_GraphVar && !(discard_prev_key & kb_GraphVar) && discards_left > 0 && hand.amt_selected > 0) {

			for (int i = 0; i < hand.hand_size; i++) {
				if (hand.hand[i].to_play) {
					hand.hand[i].value = -1;
					hand.hand[i].to_play = false;
					hand.hand[i].is_selected = false;
					hand.current_cards_cnt--;
				}
			}
			hand.amt_selected = 0;
			discards_left--;
		}
		discard_prev_key = discard_key;
		//use arrow keys to change card to select
		arrow_key = kb_Data[7];
		if (arrow_key & kb_Left && !(arrow_prev_key & kb_Left)) {
			if (card_idx > 0) {
				hand.hand[card_idx--].is_selected = false;
			}
		}
		if (arrow_key & kb_Right && !(arrow_prev_key & kb_Right)) {
			if (card_idx < hand.hand_size-1) // 52 -> deck size | 8 -> hand_size
				hand.hand[card_idx++].is_selected = false;
		}

		arrow_prev_key = arrow_key;

		//print_card(hand.hand[card_idx], 10, 10);
		display_hand(&hand);
		display_game_stats(score, target_score, hands_left, discards_left);

		if (kb_Data[6] & kb_Clear) { 
            running = false;
        }
		gfx_SwapDraw();
	}
	gfx_End();
	return 0;
}