/*
 * see:
 * github.com/braidedloaf/calcatro
 * for full repo + game files
 * 
 * credit to: 
 * localthunk
 * for balatro
 * playbalatro.com
*/

#include "card_icons_assets.h"
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


#define PI 3.14159265

unsigned int frame_timer = 0;

const int base_ante_values[8] = {300, 800, 2000, 5000, 11000, 20000, 35000, 50000};
int current_ante = 0;
int current_blind = 0; // 0 - small | 1 - big | 2 - boss

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_RULES,
	STATE_BLIND_SELECT,
	STATE_SHOP
} GameState;

typedef struct {
	char rank;
	char suit;
	int value;
	bool is_selected;
	bool to_play;
    int sprite_row;
    int sprite_col;
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

typedef struct {
	int num_points;
	int points[18];
	int is_selected;
} Planet;

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

typedef struct {
    HandValue value;
    Card scoring_cards[5];
    int scoring_count;
} EvaluatedHand;

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

typedef struct {
    int bonus_chips;
    int bonus_mult;
} HandUpgrade;

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
    [HAND_ROYAL_FLUSH]    = {0, 0}
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

int card_equal(const Card *card, const Card *drac) {
    return (card->rank == drac->rank) && (card->value == drac->value) && (card->suit == drac->suit);
}

void wait_frames(unsigned int frames) {
    for (unsigned int i = 0; i < frames; i++) {
        kb_Scan();
        gfx_Wait();
    }
}

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
		deck.cards[i].rank = (i % 13) < 8 ? '2' + (i % 13) : (i % 13) == 8 ? 'T' : (i % 13) == 9 ? 'J' : (i % 13) == 10 ? 'Q' : (i % 13) == 11 ? 'K' : 'A';
		deck.cards[i].suit = (i / 13 == 0) ? 'H' : (i / 13 == 1) ? 'C' : (i / 13 == 2) ? 'D' : 'S';
		deck.cards[i].value = (i % 13) + 2 < 10 ? (i % 13) + 2 : (i % 13) + 2 == 14 ? 11: 10;
		deck.cards[i].is_selected = false;
		deck.cards[i].to_play = false;
        deck.cards[i].sprite_row = i / 13;
        deck.cards[i].sprite_col = i % 13;
	}
	deck.size = 52;
	return deck;
}

void draw_main_menu(void) {
    gfx_FillScreen(255);
    gfx_SetTextScale(2, 2);
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("CALCATRO", 80, 40);
    gfx_SetTextScale(1, 2);
    gfx_PrintStringXY("Press [2nd] to Start", 80, 100);
    gfx_PrintStringXY("Press [alpha] for Rules", 80, 132);
    gfx_PrintStringXY("Press [Clear] to Quit", 80, 164);
}

void draw_rules_menu(void) {
    gfx_FillScreen(255);
    gfx_PrintStringXY("Press [alpha] for Menu", 80, 132);
} 

void print_card(Card c , int x, int y) {
	char left = '<';
    char right = '>';

    if (c.is_selected) {
        left = '>';
        right = '<';
    }

    if (c.is_selected || c.to_play) {
        gfx_SetTextXY(x - 8, y);
        gfx_PrintChar(left);
    }

    gfx_SetTextXY(x, y);
    if (c.value == -1) {
        gfx_PrintChar('-');
        gfx_PrintChar('-');
    } else {
        gfx_RLETSprite(card_icons_tiles[c.sprite_row*13 + c.sprite_col], x, y);
    }

    if (c.is_selected || c.to_play) {
        gfx_SetTextXY(x + 16, y);
        gfx_PrintChar(right);
    }
}

void print_playing_card(Card c, int x, int y) {
	gfx_RLETSprite(card_icons_tiles[c.sprite_row*13 + c.sprite_col], x, y);
}

void print_playing_value(int value, int x, int y) {
    gfx_SetTextXY(x-8, y);
    gfx_PrintChar('+');
    gfx_PrintInt(value, 1);
}

void print_hand_stats(HandValue hv, int x, int y) {
		gfx_SetTextXY(x, y+16); 
		gfx_PrintInt(hv.chips, 1); 
		gfx_PrintString(" x "); 
		gfx_PrintInt(hv.mult, 1);
}

void print_hand_type(HandValue hv, int x, int y, int hand_score) {
    if (hv.chips == -1) {
        gfx_SetTextXY(x, y+16);
        gfx_PrintInt(hand_score, 1);
    } else if (hv.type >= 0) {
		gfx_SetTextXY(x, y);
		switch(hv.type) {
			case HAND_HIGH_CARD:      gfx_PrintString("High Card"); print_hand_stats(hv, x, y); break;
			case HAND_ONE_PAIR:       gfx_PrintString("Pair"); print_hand_stats(hv, x, y); break;
			case HAND_TWO_PAIR:       gfx_PrintString("Two Pair"); print_hand_stats(hv, x, y); break;
			case HAND_THREE_KIND:     gfx_PrintString("Three of a Kind"); print_hand_stats(hv, x, y); break;
			case HAND_STRAIGHT:       gfx_PrintString("Straight"); print_hand_stats(hv, x, y); break;
			case HAND_FLUSH:          gfx_PrintString("Flush"); print_hand_stats(hv, x, y); break;
			case HAND_FULL_HOUSE:     gfx_PrintString("Full House"); print_hand_stats(hv, x, y); break;
			case HAND_FOUR_KIND:      gfx_PrintString("Four of a Kind"); print_hand_stats(hv, x, y); break;
			case HAND_STRAIGHT_FLUSH: gfx_PrintString("Straight Flush"); print_hand_stats(hv, x, y); break;
			case HAND_ROYAL_FLUSH:    gfx_PrintString("Royal Flush"); print_hand_stats(hv, x, y); break;
			default:                  gfx_PrintString(""); print_hand_stats(hv, x, y); break;
		}
	}
}

void display_game_stats(int score, int target_score, int hand_score, int hands_left, int discards_left, int money, HandValue hv) {
	gfx_SetTextXY(10, 2);
    gfx_SetTextScale(1, 1);
    gfx_PrintString("Score at");
    gfx_PrintStringXY("least:", 10, 10);
    gfx_SetTextScale(1, 2);
	gfx_SetTextXY(10, 20);
	gfx_PrintInt(target_score, 1);
	gfx_SetTextXY(10, 36);
    gfx_SetTextScale(1, 1);
    gfx_PrintString("Round Score:");
    gfx_SetTextScale(1, 2);
    gfx_SetTextXY(10, 44);
	gfx_PrintInt(score, 1);
	gfx_SetTextXY(10, 150);
	gfx_PrintString("H: ");
	gfx_PrintInt(hands_left, 1);
	gfx_PrintString(" D: ");
	gfx_PrintInt(discards_left, 1);
    gfx_SetTextXY(10, 166);
    gfx_PrintChar('$');
    gfx_PrintInt(money, 1);
	print_hand_type(hv, 10, 78, hand_score);
}

void draw_blind_menu(int score, int hands_left, int discards_left, int money, HandValue hv) {
    gfx_FillScreen(255);
	gfx_SetTextScale(1, 2);
	display_game_stats(score, 0, 0, hands_left, discards_left, money, hv);
	

	int offset_x = 100;
	int offset_y = 120;
	
    int box_width = 68;
    int padding_x = 10;

    for (int i = 0; i < 3; i++) {
	    gfx_PrintStringXY(current_blind == i ? "Select" : current_blind < i ? "Next" : "Passed", offset_x + ((box_width+padding_x)*i), offset_y);

        gfx_SetTextScale(1, 1);
	    gfx_PrintStringXY(i == 0 ? "Small" : i == 1 ? "Big" : "Boss", offset_x + ((box_width+padding_x)*i), offset_y + 24);
	    gfx_PrintStringXY("Blind", offset_x + ((box_width+padding_x)*i), offset_y + 32);

        gfx_SetTextScale(1, 2);
	    gfx_SetTextXY(offset_x + ((box_width+padding_x)*i), offset_y + 48);
	    gfx_PrintInt((int) (base_ante_values[current_ante] * (i*.5f + 1)), 0);

        if (current_blind == i) {
            gfx_SetTextXY(offset_x + 4 + ((box_width+padding_x)*i), offset_y-26);
            gfx_PrintString("[2nd]");
        }

        gfx_Rectangle(offset_x - padding_x + (box_width * i) + (padding_x * i), offset_y-5, box_width, GFX_LCD_HEIGHT-offset_y+5);
    }
} 

void draw_planet(Planet *planet, int offset_x, int offset_y, int pscale, int idx, double pnet[9][18], int m) {
	if (pnet[idx][0] == 8888) {
        gfx_Circle_NoClip(offset_x + 100 + (m*50), offset_y + 40, 1 * pscale);
    } else {
        for (int i = 0; i < 18; i += 2) {
            if (pnet[idx][i] == 9999 || pnet[idx][i + 1] == 9999) break;

            int x = (int) (pnet[idx][i] * pscale) + offset_x + 100 + (m*50);
            int y = (int) (pnet[idx][i + 1] * pscale) + offset_y + 40;

            planet->points[planet->num_points * 2] = x;
            planet->points[planet->num_points * 2 + 1] = y;
            planet->num_points++;
        }

        gfx_Polygon_NoClip(planet->points, planet->num_points);
    }
}

void draw_shop(int score, int hands_left, int discards_left, int *money, HandValue hv, double pnet[9][18], int pscale, int p_idx_1, int p_idx_2, bool *bought) {
    gfx_FillScreen(255);
	gfx_SetTextScale(1, 2);
	display_game_stats(score, 0, 0, hands_left, discards_left, *money, hv);

    int offset_x = 90;
    int offset_y = 115;
    int box_width = GFX_LCD_WIDTH-offset_x - 50;

    gfx_Rectangle(offset_x, offset_y, box_width, GFX_LCD_HEIGHT-offset_y);
    gfx_Rectangle(offset_x + 5, offset_y + 5, box_width / 3, (GFX_LCD_HEIGHT-offset_y) / 3);
    gfx_PrintStringXY("[alpha]", offset_x + 10, offset_y + 10);
    gfx_PrintStringXY("Next", offset_x + 10, offset_y + 26);
    
    gfx_Rectangle(offset_x + 5, offset_y + (GFX_LCD_HEIGHT-offset_y) / 3 + 5, box_width/3, (GFX_LCD_HEIGHT-offset_y) / 2);
    gfx_PrintStringXY("[del]", offset_x + 10, offset_y + (GFX_LCD_HEIGHT-offset_y) / 3 + 10);
    gfx_PrintStringXY("Reroll", offset_x + 10, offset_y + (GFX_LCD_HEIGHT-offset_y) / 3 + 26);
    gfx_SetTextXY(offset_x + 10, offset_y + (GFX_LCD_HEIGHT-offset_y) / 3 + 42);
    gfx_PrintChar('$');
    gfx_PrintInt(5, 1);

	Planet planets[2];
	memset(planets, 0, sizeof(planets));

    if (!bought[0])
        draw_planet(&planets[0], offset_x, offset_y, pscale, p_idx_1, pnet, 0);
    if (!bought[1])
        draw_planet(&planets[1], offset_x, offset_y, pscale, p_idx_2, pnet, 1);
}

void draw_pre_shop(int score, int hands_left, int discards_left, int *money, HandValue hv) {
    gfx_FillScreen(255);
	gfx_SetTextScale(1, 2);
	display_game_stats(score, 0, 0, hands_left, discards_left, *money, hv);

    int offset_x = 90;
    int offset_y = 115;
    int box_width = GFX_LCD_WIDTH-offset_x - 50;

    gfx_Rectangle(offset_x, offset_y, box_width, GFX_LCD_HEIGHT-offset_y);
    gfx_Rectangle(offset_x + 5, offset_y + 5, box_width - 10, (GFX_LCD_HEIGHT-offset_y) / 4);
    gfx_PrintStringXY("[2nd] Cash Out: $", offset_x + 10, offset_y + 10);
    gfx_PrintInt((current_blind == 0 ? 5 : current_blind == 1 ? 3 : 4) + hands_left + (*money > 25 ? 5 : *money / 5) , 1); // total money gained
}

void display_hand(Hand *p_hand) {
	for (int i = 0; i < p_hand->hand_size; i++) {

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
    char straight_ranks[] = {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};
    for (int i = 0; i <= 8; i++) {
        bool all_in_a_row = true;
        for (int j = 0; j < 5; j++) {
            if (rank_count[(int)straight_ranks[i+j]] == 0) {
                all_in_a_row = false;
                break;
            }
        }
        if (all_in_a_row) {
            is_straight = true;
            straight_high_rank = (int)straight_ranks[i+4];
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

    int pairs = 0, trips = 0, quads = 0;
    char pair_ranks[2] = {0};
    char trip_rank = 0, quad_rank = 0;

    for (char *r = "23456789TJQKA"; *r; r++) {
        int n = rank_count[(int)*r];
        if (n == 4) {
            quads++;
            quad_rank = *r;
        } else if (n == 3) {
            trips++;
            trip_rank = *r;
        } else if (n == 2) {
            if (pairs < 2)
                pair_ranks[pairs] = *r;
            pairs++;
        }
    }

    // Match cards in original order
    #define MATCH_RANKS(ranklist, num) \
        for (int i = 0; i < count; i++) { \
            for (int r = 0; r < (num); r++) { \
                if (p_cards[i].rank == (ranklist)[r]) { \
                    result.scoring_cards[result.scoring_count++] = p_cards[i]; \
                    break; \
                } \
            } \
        }

    #define MATCH_ALL() \
        for (int i = 0; i < count; i++) { \
            result.scoring_cards[result.scoring_count++] = p_cards[i]; \
        }

    #define MATCH_FLUSH(f_suit) \
        for (int i = 0; i < count; i++) { \
            if (p_cards[i].suit == (f_suit)) { \
                result.scoring_cards[result.scoring_count++] = p_cards[i]; \
            } \
        }

    if (count == 5 && is_straight && is_flush && straight_high_rank == 'A') {
        result.value = hand_table[HAND_ROYAL_FLUSH];
        MATCH_ALL();
        return result;
    }

    if (count == 5 && is_straight && is_flush) {
        result.value = hand_table[HAND_STRAIGHT_FLUSH];
        MATCH_ALL();
        return result;
    }

    if (quads) {
        result.value = hand_table[HAND_FOUR_KIND];
        MATCH_RANKS(&quad_rank, 1);
        return result;
    }

    if (trips && pairs) {
        result.value = hand_table[HAND_FULL_HOUSE];
        char ranks[2] = { trip_rank, pair_ranks[0] };
        MATCH_RANKS(ranks, 2);
        return result;
    }

    if (is_flush) {
        result.value = hand_table[HAND_FLUSH];
        MATCH_FLUSH(flush_suit);
        return result;
    }

    if (is_straight) {
        result.value = hand_table[HAND_STRAIGHT];
        MATCH_ALL();
        return result;
    }

    if (trips) {
        result.value = hand_table[HAND_THREE_KIND];
        MATCH_RANKS(&trip_rank, 1);
        return result;
    }

    if (pairs >= 2) {
        result.value = hand_table[HAND_TWO_PAIR];
        MATCH_RANKS(pair_ranks, 2);
        return result;
    }

    if (pairs == 1) {
        result.value = hand_table[HAND_ONE_PAIR];
        MATCH_RANKS(&pair_ranks[0], 1);
        return result;
    }

    result.value = hand_table[HAND_HIGH_CARD];

    Card highest = cards[count - 1];

    for (int i = 0; i < count; i++) {
        if (card_equal(&p_cards[i], &highest)) {
            result.scoring_cards[0] = p_cards[i];
            result.scoring_count = 1;
            break;
        }
    }

    return result;

    #undef MATCH_RANKS
    #undef MATCH_ALL
    #undef MATCH_FLUSH
}

void display_playing_hand(Card *cards, int count, Card *scoring_cards, int pos) {
    int offset_x = 120;
    int offset_y = 100;
	for (int i = 0; i < count; i++) {
        print_playing_card(cards[i], offset_x + (i*32), offset_y);
        if (pos >= 0 && card_equal(&cards[i], &scoring_cards[pos])) {
            print_playing_value(scoring_cards[pos].value, offset_x + (i*32), offset_y - 24);
        }
	}
}

int handle_draw_scoring(Card *all_cards, int all_cards_count, Card *cards, int count, const HandValue *hand_type, Hand *p_hand, int score, int target_score, int hands_left, int discards_left, int money) {
    int base = hand_type->chips;
    int running_total = base;

    for (int i = 0; i < count; i++) {
        running_total += cards[i].value;

        gfx_FillScreen(255);
        display_hand(p_hand);

        HandValue display = *hand_type;
        display.chips = running_total;
		display_playing_hand(all_cards, all_cards_count, cards, i);
        display_game_stats(score, target_score, 0, hands_left, discards_left, money, display);
        gfx_SwapDraw();

		
        wait_frames(250);
    }


    return running_total * hand_type->mult;
}

int main(void) {
	srand(time(NULL));

    GameState state = STATE_MENU;
	int running = 1;

    gfx_Begin();
    gfx_SetPalette(card_palette, sizeof_card_palette, 0);
	
goto_menu:
    while (state == STATE_MENU) {
        kb_Scan();
        draw_main_menu();
        gfx_SwapDraw();

        if (kb_Data[1] & kb_2nd) {
            while (kb_Data[1] & kb_2nd) kb_Scan();
            state = STATE_BLIND_SELECT;
        } else if (kb_Data[2] & kb_Alpha) {
            while (kb_Data[2] & kb_Alpha) kb_Scan();
            state = STATE_RULES;
        } else if (kb_Data[6] & kb_Clear) {
            gfx_End();
            return 0;
        }
		
		
        gfx_Wait();
    }
	
	int score = 0, target_score = 0, hands_left = 4, discards_left = 3, money = 4;
	kb_key_t arrow_key, arrow_prev_key = 0, select_key, select_prev_key = 0, discard_key, discard_prev_key = 0, play_key, play_prev_key = 0, p_idx_1, p_idx_2;
    int planet_scale = 20;
	double planet_shapes[9][18] = {
        //Circle sentinel
        {8888, 8888, 0,0, 2,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999},
        // Triangle
        { 1.000000, 0.000000, -0.500000, 0.866025, -0.500000, -0.866025, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999},
        // Square
        {-1,1, 1,1, 1,-1, -1,-1, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999},
        // Trapezoid
        { -1.000000, 0.500000, 1.000000, 0.500000, 0.500000, -1.00000, -0.500000, -1.000000, 9999,9999, 9999,9999, 9999,9999, 9999,9999, 9999,9999},
        // Pentagon
        {1.000000, 0.000000, 0.309017, 0.951057, -0.809017, 0.587785, -0.809017, -0.587785, 0.309017, -0.951057, 9999,9999, 9999,9999, 9999,9999, 9999,9999},
        // Hexagon
        {1.000000, 0.000000, 0.500000, 0.866025, -0.500000, 0.866025, -1.000000, 0.000000, -0.500000, -0.866025, 0.500000, -0.866025, 9999,9999, 9999,9999, 9999,9999},
        // Septagon
        { 1.000000, 0.000000, 0.623490, 0.781831, -0.222521, 0.974928, -0.900969, 0.433884, -0.900969, -0.433884, -0.222521, -0.974928, 0.623490, -0.781831, 9999,9999 },
        // Octagon
        { 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 1.000000, -0.707107, 0.707107, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.707107, -0.707107, 9999,9999 },
        // Nonagon
        { 1.000000, 0.000000, 0.766044, 0.642788, 0.173648, 0.984808, -0.500000, 0.866025, -0.939693, 0.342020, -0.939693, -0.342020, -0.500000, -0.866025, 0.173648, -0.984808, 0.766044, -0.642788 }
    };
goto_blind_select:
    while (state == STATE_BLIND_SELECT) {
		kb_Scan();
        draw_blind_menu(score, hands_left, discards_left, money, (HandValue){-1,0,0});
        gfx_SwapDraw();
		
		if (kb_Data[1] & kb_2nd) {
            while (kb_Data[1] & kb_2nd) kb_Scan();
            state = STATE_GAME;
            target_score = (int) (base_ante_values[current_ante] * (current_blind*.5f + 1));
        } else if (kb_Data[6] & kb_Clear) {
            gfx_End();
            return 0;
        }
	}

goto_shop:
	p_idx_1 = randomIntRange(0, 8);
    p_idx_2 = randomIntRange(0, 8);
    while (p_idx_1 == p_idx_2) p_idx_2 = randomIntRange(0,8);
    int pre_shop = 1;
    bool bought[2] = { false, false };
    while (state == STATE_SHOP) {
        while (pre_shop) {
            kb_Scan();
            draw_pre_shop(score, hands_left, discards_left, &money, (HandValue) {-1, 0, 0});
            gfx_SwapDraw();
        
            if (kb_Data[1] & kb_2nd) {
                while (kb_Data[1] & kb_2nd) kb_Scan();
                money += (current_blind == 0 ? 5 : current_blind == 1 ? 3 : 4) + hands_left + money / 5;
                pre_shop = 0;
                break;
            } else if (kb_Data[6] & kb_Clear) {
                gfx_End();
                return 0;
            }
        }

        hands_left = 4;
        discards_left = 3;

        kb_Scan();
        draw_shop(score, hands_left, discards_left, &money, (HandValue) {-1, 0, 0}, planet_shapes, planet_scale, p_idx_1, p_idx_2, bought);
        gfx_SwapDraw();

        if ((kb_Data[1] & kb_2nd) && money >= 3 && !bought[0]) {
            HandType ht = shape_hand_map[p_idx_1];
            money -= 3;
            bought[0] = true;
            hand_table[ht].chips += upgrade_table[ht].bonus_chips;
            hand_table[ht].mult  += upgrade_table[ht].bonus_mult;

            if (ht == HAND_STRAIGHT_FLUSH) {
                hand_table[HAND_ROYAL_FLUSH].chips += upgrade_table[ht].bonus_chips;
                hand_table[HAND_ROYAL_FLUSH].mult  += upgrade_table[ht].bonus_mult;
            }
        }

        if ((kb_Data[3] & kb_GraphVar) && money >= 3 && !bought[1]) {
            HandType ht = shape_hand_map[p_idx_2];
            money -= 3;
            bought[1] = true;
            hand_table[ht].chips += upgrade_table[ht].bonus_chips;
            hand_table[ht].mult  += upgrade_table[ht].bonus_mult;

            if (ht == HAND_STRAIGHT_FLUSH) {
                hand_table[HAND_ROYAL_FLUSH].chips += upgrade_table[ht].bonus_chips;
                hand_table[HAND_ROYAL_FLUSH].mult  += upgrade_table[ht].bonus_mult;
            }
        }
        
        if ((kb_Data[1] & kb_Del) && money >= 5) {
            while (kb_Data[1] & kb_Del) kb_Scan();
            money -= 5;

            do {
                p_idx_1 = randomIntRange(0, 8);
                p_idx_2 = randomIntRange(0, 8);
            } while (p_idx_1 == p_idx_2);

            bought[0] = false;
            bought[1] = false;
        }

        if (kb_Data[2] & kb_Alpha) {
            while (kb_Data[2] & kb_Alpha) kb_Scan();
            state = STATE_BLIND_SELECT;
            goto goto_blind_select;
        } else if (kb_Data[6] & kb_Clear) {
            gfx_End();
            return 0;
        }
    }

	
    while (state == STATE_RULES) {
        kb_Scan();
        draw_rules_menu();
        gfx_SwapDraw();

        if (kb_Data[6] & kb_Clear) {
            gfx_End();
            return 0;
        } else if (kb_Data[2] & kb_Alpha) {
            while (kb_Data[2] & kb_Alpha) kb_Scan();
            state = STATE_MENU;
            goto goto_menu;
        } 
        gfx_Wait();
    }

	
	gfx_SetTextScale(1,2);
	gfx_SetColor(0);
    
    //create deck every round
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
	
	while (running && state == STATE_GAME) {
		gfx_FillScreen(255);
		frame_timer++;
		kb_Scan();
		
		
		if (hand.current_cards_cnt < hand.hand_size) {
			draw_cards_to_hand(&hand, hand.hand_size - hand.current_cards_cnt, &deck, &next_card);
		}
		
        //handle selection
		select_key = kb_Data[1];
		hand.hand[card_idx].is_selected = true;
		if ((select_key & kb_2nd) && !(select_prev_key & kb_2nd)) {
			if (!hand.hand[card_idx].to_play && hand.amt_selected < 5) {
				hand.hand[card_idx].to_play = true;
				hand.amt_selected++;
			} else if (hand.hand[card_idx].to_play) {
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

		EvaluatedHand result = get_hand_type(playing_hand, playing_hand_idx);
		
        //handle discarding
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

        //handle playing
		play_key = kb_Data[2];
		if (play_key & kb_Alpha && !(play_prev_key & kb_Alpha) && hands_left > 0 && hand.amt_selected > 0) {
			int tc = 0;
			for (int i = 0; i < hand.hand_size; i++) {
				if (hand.hand[i].to_play) {
					hand.hand[i].value = -1;
					hand.hand[i].to_play = false;
					hand.hand[i].is_selected = false;
					tc++;
				}
			}
			hand.amt_selected = 0;
			hands_left--;

            //pre scoring
            gfx_FillScreen(255);
            display_hand(&hand);
            display_playing_hand(playing_hand, playing_hand_idx, result.scoring_cards, -1);
            display_game_stats(score, target_score, 0, hands_left, discards_left, money, result.value);
            gfx_SwapDraw();
            wait_frames(500);
            //scoring
			int gained = handle_draw_scoring(playing_hand, playing_hand_idx, result.scoring_cards, result.scoring_count, &result.value, &hand, score, target_score, hands_left, discards_left, money);

            HandValue post_score_value = result.value;
            post_score_value.chips = gained / result.value.mult;
            post_score_value.mult = result.value.mult;

            gfx_FillScreen(255);
            display_hand(&hand);
            display_playing_hand(playing_hand, playing_hand_idx, result.scoring_cards, -1);
            display_game_stats(score, target_score, gained, hands_left, discards_left, money, post_score_value);
            gfx_SwapDraw();
            wait_frames(500);

            gfx_FillScreen(255);
            display_hand(&hand);
            display_playing_hand(playing_hand, playing_hand_idx, result.scoring_cards, -1);
            display_game_stats(score, target_score, gained, hands_left, discards_left, money, (HandValue){-1,-1,0});
            gfx_SwapDraw();
            wait_frames(750);
            
            score += gained;

			hand.current_cards_cnt -= tc;
            
            //beaten blind
            
		}
		play_prev_key = play_key;

		//use arrow keys to change card to select
		arrow_key = kb_Data[7];
		if (arrow_key & kb_Left && !(arrow_prev_key & kb_Left)) {
            if (card_idx == 0) {
                hand.hand[card_idx].is_selected = false;
                card_idx = hand.hand_size-1;
            } else if (card_idx > 0) 
				hand.hand[card_idx--].is_selected = false;
		}
		if (arrow_key & kb_Right && !(arrow_prev_key & kb_Right)) {
            if (card_idx == hand.hand_size-1) {
                hand.hand[card_idx].is_selected = false;
                card_idx = 0;
            } else if (card_idx < hand.hand_size-1) // 52 -> deck size | 8 -> hand_size
				hand.hand[card_idx++].is_selected = false;
		}

		arrow_prev_key = arrow_key;

		display_hand(&hand);
		display_game_stats(score, target_score, 0, hands_left, discards_left, money, result.value);

        if (score >= target_score) {
            score = 0;
            if (current_blind == 2) { // beat boss blind
                current_blind = 0;
                current_ante++;
            } else {
                current_blind++;
            }
            state = STATE_SHOP;
            goto goto_shop;
        }

		if (kb_Data[6] & kb_Clear) { 
            running = false;
        }
		gfx_SwapDraw();
	}
	gfx_End();
	return 0;
}