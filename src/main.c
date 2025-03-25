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
    HAND_COUNT
} HandType;

typedef struct {
    HandType type;
    int chips;
    int mult;
} HandValue;

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
    { HAND_ROYAL_FLUSH,    100, 8 }
};

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

void print_hand_type(HandValue hv, int x, int y) {
    gfx_SetTextXY(x, y);
    switch (hv.type) {
        case HAND_HIGH_CARD:      gfx_PrintString("High Card"); break;
        case HAND_ONE_PAIR:       gfx_PrintString("Pair"); break;
        case HAND_TWO_PAIR:       gfx_PrintString("Two Pair"); break;
        case HAND_THREE_KIND:     gfx_PrintString("Three of a Kind"); break;
        case HAND_STRAIGHT:       gfx_PrintString("Straight"); break;
        case HAND_FLUSH:          gfx_PrintString("Flush"); break;
        case HAND_FULL_HOUSE:     gfx_PrintString("Full House"); break;
        case HAND_FOUR_KIND:      gfx_PrintString("Four of a Kind"); break;
        case HAND_STRAIGHT_FLUSH: gfx_PrintString("Straight Flush"); break;
        case HAND_ROYAL_FLUSH:    gfx_PrintString("Royal Flush"); break;
        default:                  gfx_PrintString(""); break;
    }
}

void display_game_stats(int score, int target_score, int hands_left, int discards_left, HandValue hv) {
	gfx_SetTextXY(10, 20);
	gfx_PrintInt(target_score, 1);
	gfx_SetTextXY(10, 36);
	gfx_PrintInt(score, 1);
	gfx_SetTextXY(10, 52);
	gfx_PrintString("H: ");
	gfx_PrintInt(hands_left, 1);
	gfx_PrintString(" D: ");
	gfx_PrintInt(discards_left, 1);
	print_hand_type(hv, 10, 78);
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

HandValue get_hand_type(Card *cards, int count) {
	if (count < 2 || count > 5)
		return hand_table[HAND_HIGH_CARD];

	// Sort by value
	for (int i = 0; i < count - 1; i++) {
		for (int j = i + 1; j < count; j++) {
			if (cards[i].value > cards[j].value) {
				Card tmp = cards[i];
				cards[i] = cards[j];
				cards[j] = tmp;
			}
		}
	}

	int rank_count[15] = {0}; // ranks 2–14
	int suit_count[4] = {0};  // S, H, C, D

	for (int i = 0; i < count; i++) {
		rank_count[cards[i].value]++;
		switch (cards[i].suit) {
			case 'S': suit_count[0]++; break;
			case 'H': suit_count[1]++; break;
			case 'C': suit_count[2]++; break;
			case 'D': suit_count[3]++; break;
		}
	}

	// Detect flush
	bool is_flush = false;
	for (int i = 0; i < 4; i++) {
		if (suit_count[i] == count) {
			is_flush = true;
			break;
		}
	}

	// Detect straight
	bool is_straight = false;
	int high_card = 0;
	for (int i = 2; i <= 14 - (count - 1); i++) {
		bool all_in_a_row = true;
		for (int j = 0; j < count; j++) {
			if (rank_count[i + j] == 0) {
				all_in_a_row = false;
				break;
			}
		}
		if (all_in_a_row) {
			is_straight = true;
			high_card = i + count - 1;
			break;
		}
	}
	// Special case: A-2-3-4-5
	if (!is_straight && count == 5 &&
	    rank_count[14] && rank_count[2] && rank_count[3] && rank_count[4] && rank_count[5]) {
		is_straight = true;
		high_card = 5;
	}

	// Count multiples
	int pairs = 0, trips = 0, quads = 0;
	for (int i = 2; i <= 14; i++) {
		if (rank_count[i] == 4) quads++;
		else if (rank_count[i] == 3) trips++;
		else if (rank_count[i] == 2) pairs++;
	}

	// Detect hands by count
	if (count == 5) {
		if (is_straight && is_flush && high_card == 14)
			return hand_table[HAND_ROYAL_FLUSH];
		if (is_straight && is_flush)
			return hand_table[HAND_STRAIGHT_FLUSH];
		if (quads)
			return hand_table[HAND_FOUR_KIND];
		if (trips && pairs)
			return hand_table[HAND_FULL_HOUSE];
		if (is_flush)
			return hand_table[HAND_FLUSH];
		if (is_straight)
			return hand_table[HAND_STRAIGHT];
	}

	// Apply to all hand sizes 2–5
	if (trips)
		return hand_table[HAND_THREE_KIND];
	if (pairs == 2)
		return hand_table[HAND_TWO_PAIR];
	if (pairs == 1)
		return hand_table[HAND_ONE_PAIR];

	return hand_table[HAND_HIGH_CARD];
}

int handle_scoring(Card *cards, HandValue hand_type, int count) {
	return 0;
}

int main(void) {
	srand(time(NULL));

	kb_key_t arrow_key, arrow_prev_key = 0, select_key, select_prev_key = 0, discard_key, discard_prev_key = 0, play_key, play_prev_key = 0;
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

		Card playing_hand[5]; 
		int playing_hand_idx = 0;
		for (int i = 0; i < hand.hand_size; i++) {
			if (hand.hand[i].to_play) {
				playing_hand[playing_hand_idx++] = hand.hand[i];
			}
		}

		HandValue hand_type = get_hand_type(playing_hand, playing_hand_idx);
		
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

		play_key = kb_Data[2];
		if (play_key & kb_Alpha && !(play_prev_key & kb_Alpha) && hands_left > 0 && hand.amt_selected > 0) {
			for (int i = 0; i < hand.hand_size; i++) {
				if (hand.hand[i].to_play) {
					hand.hand[i].value = -1;
					hand.hand[i].to_play = false;
					hand.hand[i].is_selected = false;
					hand.current_cards_cnt--;
				}
			}
			hand.amt_selected = 0;
			hands_left--;
			//handle_scoring(playing_hand, hand_type, playing_hand_idx);
		}
		play_prev_key = play_key;

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
		display_game_stats(score, target_score, hands_left, discards_left, hand_type);

		if (kb_Data[6] & kb_Clear) { 
            running = false;
        }
		gfx_SwapDraw();
	}
	gfx_End();
	return 0;
}