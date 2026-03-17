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
#include "deck.h"
#include "game.h"
#include "gameplay.h"
#include "hand_eval.h"
#include "joker.h"
#include "ui.h"
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

unsigned int frame_timer = 0;

const int base_ante_values[8] = {300, 800, 2000, 5000, 11000, 20000, 35000, 50000};

int randomIntRange(int min, int max) {
	return min + (rand() % (max-min+1));
}

static void wait_for_key_release(void) {
    do {
        kb_Scan();
    } while (kb_AnyKey());
}

static JokerType random_joker_type(void) {
    return (JokerType)randomIntRange(JOKER_BASIC_CHIPS, JOKER_PAIR_MULT);
}

static void roll_shop_items(ShopItem *items) {
    HandType first_planet = shape_hand_map[randomIntRange(0, 8)];
    HandType second_planet = shape_hand_map[randomIntRange(0, 8)];

    while (second_planet == first_planet) {
        second_planet = shape_hand_map[randomIntRange(0, 8)];
    }

    items[0].type = SHOP_ITEM_PLANET;
    items[0].cost = 3;
    items[0].bought = false;
    items[0].data.planet_hand = first_planet;

    items[1].type = SHOP_ITEM_PLANET;
    items[1].cost = 3;
    items[1].bought = false;
    items[1].data.planet_hand = second_planet;

    items[2].type = SHOP_ITEM_JOKER;
    items[2].cost = 5;
    items[2].bought = false;
    items[2].data.joker_type = random_joker_type();
}

static bool is_shop_slot_selectable(int slot, const ShopItem *items) {
    if (slot >= 0 && slot < SHOP_ITEM_COUNT) {
        return !items[slot].bought;
    }

    return true;
}

static int move_shop_focus(int focused_slot, int direction, const ShopItem *items) {
    int next_slot = focused_slot;
    const int slot_count = SHOP_ITEM_COUNT + 2;

    do {
        next_slot = (next_slot + direction + slot_count) % slot_count;
    } while (next_slot != focused_slot && !is_shop_slot_selectable(next_slot, items));

    return next_slot;
}

static GameState handle_menu_state(bool *quit) {
    while (true) {
        kb_Scan();
        draw_main_menu();
        gfx_SwapDraw();

        if (kb_Data[1] & kb_2nd) {
            wait_for_key_release();
            return STATE_BLIND_SELECT;
        }
        if (kb_Data[2] & kb_Alpha) {
            wait_for_key_release();
            return STATE_RULES;
        }
        if (kb_Data[6] & kb_Clear) {
            *quit = true;
            return STATE_MENU;
        }

        gfx_Wait();
    }
}

static GameState handle_rules_state(bool *quit) {
    while (true) {
        kb_Scan();
        draw_rules_menu();
        gfx_SwapDraw();

        if (kb_Data[6] & kb_Clear) {
            *quit = true;
            return STATE_RULES;
        }
        if (kb_Data[2] & kb_Alpha) {
            wait_for_key_release();
            return STATE_MENU;
        }

        gfx_Wait();
    }
}

static GameState handle_blind_select_state(RunState *run_state, bool *quit) {
    while (true) {
        kb_Scan();
        draw_blind_menu(run_state->score, run_state->hands_left, run_state->discards_left, run_state->money, (HandValue){-1, 0, 0}, run_state->current_blind, run_state->current_blind, run_state->current_ante, base_ante_values);
        draw_blind_footer();
        gfx_SwapDraw();

        if (kb_Data[1] & kb_2nd) {
            wait_for_key_release();
            run_state->target_score = (int)(base_ante_values[run_state->current_ante] * (run_state->current_blind * .5f + 1));
            return STATE_GAME;
        }
        if (kb_Data[6] & kb_Clear) {
            *quit = true;
            return STATE_BLIND_SELECT;
        }
    }
}

static GameState handle_shop_state(RunState *run_state, bool *quit) {
    ShopItem items[SHOP_ITEM_COUNT];
    bool pre_shop = true;
    int focused_slot = 0;
    kb_key_t prev_arrow = 0;
    roll_shop_items(items);

    while (true) {
        while (pre_shop) {
            kb_Scan();
            draw_pre_shop(run_state->score, run_state->hands_left, run_state->discards_left, run_state->money, (HandValue){-1, 0, 0}, run_state->current_blind, true);
            draw_shop_footer(true);
            gfx_SwapDraw();

            if (kb_Data[1] & kb_2nd) {
                wait_for_key_release();
                run_state->money += (run_state->current_blind == 0 ? 5 : run_state->current_blind == 1 ? 3 : 4) + run_state->hands_left + run_state->money / 5;
                pre_shop = false;
                break;
            }
            if (kb_Data[6] & kb_Clear) {
                *quit = true;
                return STATE_SHOP;
            }
        }

        run_state->hands_left = 4;
        run_state->discards_left = 3;

        kb_Scan();
        if ((kb_Data[7] & kb_Left) && !(prev_arrow & kb_Left)) {
            focused_slot = move_shop_focus(focused_slot, -1, items);
        }
        if ((kb_Data[7] & kb_Right) && !(prev_arrow & kb_Right)) {
            focused_slot = move_shop_focus(focused_slot, 1, items);
        }
        prev_arrow = kb_Data[7];

        draw_shop(run_state->score, run_state->hands_left, run_state->discards_left, run_state->money, (HandValue){-1, 0, 0}, items, focused_slot);
        draw_shop_footer(false);
        gfx_SwapDraw();

        if ((kb_Data[1] & kb_2nd) && focused_slot >= 0 && focused_slot < SHOP_ITEM_COUNT &&
            run_state->money >= items[focused_slot].cost && !items[focused_slot].bought) {
            wait_for_key_release();
            if (items[focused_slot].type == SHOP_ITEM_PLANET) {
                HandType hand_type = items[focused_slot].data.planet_hand;
                run_state->money -= items[focused_slot].cost;
                items[focused_slot].bought = true;
                hand_table[hand_type].chips += upgrade_table[hand_type].bonus_chips;
                hand_table[hand_type].mult += upgrade_table[hand_type].bonus_mult;

                if (hand_type == HAND_STRAIGHT_FLUSH) {
                    hand_table[HAND_ROYAL_FLUSH].chips += upgrade_table[hand_type].bonus_chips;
                    hand_table[HAND_ROYAL_FLUSH].mult += upgrade_table[hand_type].bonus_mult;
                }
            } else if (items[focused_slot].type == SHOP_ITEM_JOKER &&
                       add_joker(run_state, items[focused_slot].data.joker_type, items[focused_slot].cost)) {
                run_state->money -= items[focused_slot].cost;
                items[focused_slot].bought = true;
            }

            focused_slot = move_shop_focus(focused_slot, 1, items);
        }

        if ((kb_Data[1] & kb_2nd) && focused_slot == 4 && run_state->money >= 5) {
            wait_for_key_release();
            run_state->money -= 5;
            roll_shop_items(items);
            focused_slot = 0;
        }

        if ((kb_Data[1] & kb_2nd) && focused_slot == 3) {
            wait_for_key_release();
            return STATE_BLIND_SELECT;
        }
        if (kb_Data[6] & kb_Clear) {
            *quit = true;
            return STATE_SHOP;
        }
    }
}

int main(void) {
    srand(time(NULL));

    GameState state = STATE_MENU;
    RunState run_state;
    bool quit = false;

    gfx_Begin();
    gfx_SetPalette(card_palette, sizeof_card_palette, 0);
    init_run_state(&run_state);

    while (!quit) {
        switch (state) {
            case STATE_MENU:
                state = handle_menu_state(&quit);
                break;
            case STATE_RULES:
                state = handle_rules_state(&quit);
                break;
            case STATE_BLIND_SELECT:
                state = handle_blind_select_state(&run_state, &quit);
                break;
            case STATE_SHOP:
                state = handle_shop_state(&run_state, &quit);
                break;
            case STATE_GAME:
                state = handle_game_state(&run_state, &quit, &frame_timer);
                break;
        }
    }

    gfx_End();
    return 0;
}
