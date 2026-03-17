#include "ui.h"

#include "card_icons_assets.h"
#include "hand_eval.h"
#include "planet_icons_assets.h"

#include <graphx.h>
#include <string.h>

static const uint8_t hand_type_planet_map[HAND_COUNT] = {
    [HAND_HIGH_CARD] = 0,
    [HAND_ONE_PAIR] = 1,
    [HAND_TWO_PAIR] = 3,
    [HAND_THREE_KIND] = 2,
    [HAND_STRAIGHT] = 5,
    [HAND_FLUSH] = 4,
    [HAND_FULL_HOUSE] = 6,
    [HAND_FOUR_KIND] = 7,
    [HAND_STRAIGHT_FLUSH] = 8,
    [HAND_ROYAL_FLUSH] = 8,
    [HAND_FIVE_KIND] = 9,
    [HAND_FLUSH_HOUSE] = 10,
    [HAND_FLUSH_FIVE] = 11
};

static void print_card(Card card, int x, int y) {
    char left = '<';
    char right = '>';

    if (card.is_selected) {
        left = '>';
        right = '<';
    }

    if (card.is_selected || card.to_play) {
        gfx_SetTextXY(x - 8, y);
        gfx_PrintChar(left);
    }

    gfx_SetTextXY(x, y);
    if (card.value == -1) {
        gfx_PrintChar('-');
        gfx_PrintChar('-');
    } else {
        gfx_RLETSprite(card_icons_tiles[card.sprite_row * 13 + card.sprite_col], x, y);
    }

    if (card.is_selected || card.to_play) {
        gfx_SetTextXY(x + 16, y);
        gfx_PrintChar(right);
    }
}

static void draw_focus_box(int x, int y, int width, int height) {
    gfx_Rectangle(x - 2, y - 2, width + 4, height + 4);
    gfx_Rectangle(x - 3, y - 3, width + 6, height + 6);
}

static void print_playing_card(Card card, int x, int y) {
    gfx_RLETSprite(card_icons_tiles[card.sprite_row * 13 + card.sprite_col], x, y);
}

static void print_playing_value(int value, int x, int y) {
    gfx_SetTextXY(x - 8, y);
    gfx_PrintChar('+');
    gfx_PrintInt(value, 1);
}

static void print_hand_stats(HandValue hand_value, int x, int y) {
    gfx_SetTextXY(x, y + 16);
    gfx_PrintInt(hand_value.chips, 1);
    gfx_PrintString(" x ");
    gfx_PrintInt(hand_value.mult, 1);
}

static void print_hand_type(HandValue hand_value, int x, int y, int hand_score) {
    if (hand_value.chips == -1) {
        gfx_SetTextXY(x, y + 16);
        gfx_PrintInt(hand_score, 1);
    } else if (hand_value.type >= 0) {
        gfx_SetTextXY(x, y);
        switch (hand_value.type) {
            case HAND_HIGH_CARD:      gfx_PrintString("High Card"); print_hand_stats(hand_value, x, y); break;
            case HAND_ONE_PAIR:       gfx_PrintString("Pair"); print_hand_stats(hand_value, x, y); break;
            case HAND_TWO_PAIR:       gfx_PrintString("Two Pair"); print_hand_stats(hand_value, x, y); break;
            case HAND_THREE_KIND:     gfx_PrintString("Three of a Kind"); print_hand_stats(hand_value, x, y); break;
            case HAND_STRAIGHT:       gfx_PrintString("Straight"); print_hand_stats(hand_value, x, y); break;
            case HAND_FLUSH:          gfx_PrintString("Flush"); print_hand_stats(hand_value, x, y); break;
            case HAND_FULL_HOUSE:     gfx_PrintString("Full House"); print_hand_stats(hand_value, x, y); break;
            case HAND_FOUR_KIND:      gfx_PrintString("Four of a Kind"); print_hand_stats(hand_value, x, y); break;
            case HAND_STRAIGHT_FLUSH: gfx_PrintString("Straight Flush"); print_hand_stats(hand_value, x, y); break;
            case HAND_ROYAL_FLUSH:    gfx_PrintString("Royal Flush"); print_hand_stats(hand_value, x, y); break;
            case HAND_FIVE_KIND:      gfx_PrintString("Five of a Kind"); print_hand_stats(hand_value, x, y); break;
            case HAND_FLUSH_HOUSE:    gfx_PrintString("Flush House"); print_hand_stats(hand_value, x, y); break;
            case HAND_FLUSH_FIVE:     gfx_PrintString("Flush Five"); print_hand_stats(hand_value, x, y); break;
            default:                  gfx_PrintString(""); print_hand_stats(hand_value, x, y); break;
        }
    }
}

static const char *hand_type_label(HandType type) {
    switch (type) {
        case HAND_HIGH_CARD: return "High Card";
        case HAND_ONE_PAIR: return "Pair";
        case HAND_TWO_PAIR: return "Two Pair";
        case HAND_THREE_KIND: return "Three Kind";
        case HAND_STRAIGHT: return "Straight";
        case HAND_FLUSH: return "Flush";
        case HAND_FULL_HOUSE: return "Full House";
        case HAND_FOUR_KIND: return "Four Kind";
        case HAND_STRAIGHT_FLUSH: return "Straight+";
        case HAND_ROYAL_FLUSH: return "Royal";
        case HAND_FIVE_KIND: return "Five Kind";
        case HAND_FLUSH_HOUSE: return "Flush House";
        case HAND_FLUSH_FIVE: return "Flush Five";
        default: return "";
    }
}

static void draw_planet_sprite(HandType type, int x, int y) {
    gfx_RLETSprite(planet_icons_tiles[hand_type_planet_map[type]], x, y);
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

void display_game_stats(int score, int target_score, int hand_score, int hands_left, int discards_left, int money, HandValue hand_value) {
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
    print_hand_type(hand_value, 10, 78, hand_score);
}

void draw_blind_menu(int score, int hands_left, int discards_left, int money, HandValue hand_value, int current_blind, int focused_blind, int current_ante, const int *base_ante_values) {
    gfx_FillScreen(255);
    gfx_SetTextScale(1, 2);
    display_game_stats(score, 0, 0, hands_left, discards_left, money, hand_value);

    int offset_x = 100;
    int offset_y = 120;
    int box_width = 68;
    int padding_x = 10;

    for (int i = 0; i < 3; i++) {
        gfx_PrintStringXY(current_blind == i ? "Select" : current_blind < i ? "Next" : "Passed", offset_x + ((box_width + padding_x) * i), offset_y);
        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY(i == 0 ? "Small" : i == 1 ? "Big" : "Boss", offset_x + ((box_width + padding_x) * i), offset_y + 24);
        gfx_PrintStringXY("Blind", offset_x + ((box_width + padding_x) * i), offset_y + 32);
        gfx_SetTextScale(1, 2);
        gfx_SetTextXY(offset_x + ((box_width + padding_x) * i), offset_y + 48);
        gfx_PrintInt((int)(base_ante_values[current_ante] * (i * .5f + 1)), 0);

        gfx_Rectangle(offset_x - padding_x + (box_width * i) + (padding_x * i), offset_y - 5, box_width, GFX_LCD_HEIGHT - offset_y + 5);
        if (focused_blind == i) {
            draw_focus_box(offset_x - padding_x + (box_width * i) + (padding_x * i), offset_y - 5, box_width, GFX_LCD_HEIGHT - offset_y + 5);
        }
    }
}

void draw_shop(int score, int hands_left, int discards_left, int money, HandValue hand_value, int planet_index_1, int planet_index_2, const bool *bought, int focused_slot) {
    gfx_FillScreen(255);
    gfx_SetTextScale(1, 2);
    display_game_stats(score, 0, 0, hands_left, discards_left, money, hand_value);

    int offset_x = 90;
    int offset_y = 115;
    int box_width = GFX_LCD_WIDTH - offset_x - 50;
    int utility_width = box_width / 3;
    int item_box_width = 44;
    int item_box_height = 52;
    int item_gap = 8;
    int item_x = offset_x + utility_width + 14;
    int item_y = offset_y + 8;
    gfx_Rectangle(offset_x, offset_y, box_width, GFX_LCD_HEIGHT - offset_y);
    gfx_PrintStringXY("Shop", offset_x + 8, offset_y - 12);

    gfx_Rectangle(offset_x + 5, offset_y + 5, utility_width, 24);
    gfx_PrintStringXY("Next", offset_x + 16, offset_y + 14);
    if (focused_slot == 2) {
        draw_focus_box(offset_x + 5, offset_y + 5, utility_width, 24);
    }

    gfx_Rectangle(offset_x + 5, offset_y + 35, utility_width, 36);
    gfx_PrintStringXY("Reroll", offset_x + 12, offset_y + 42);
    gfx_SetTextXY(offset_x + 22, offset_y + 56);
    gfx_PrintChar('$');
    gfx_PrintInt(5, 1);
    if (focused_slot == 3) {
        draw_focus_box(offset_x + 5, offset_y + 35, utility_width, 36);
    }

    gfx_Rectangle(item_x, item_y, item_box_width, item_box_height);
    gfx_Rectangle(item_x + item_box_width + item_gap, item_y, item_box_width, item_box_height);

    if (!bought[0]) {
        draw_planet_sprite(shape_hand_map[planet_index_1], item_x + 2, item_y + 6);
        if (focused_slot == 0) {
            draw_focus_box(item_x, item_y, item_box_width, item_box_height);
        }
    } else {
        gfx_HorizLine(item_x + 8, item_y + (item_box_height / 2), item_box_width - 16);
    }
    if (!bought[1]) {
        draw_planet_sprite(shape_hand_map[planet_index_2], item_x + item_box_width + item_gap + 2, item_y + 6);
        if (focused_slot == 1) {
            draw_focus_box(item_x + item_box_width + item_gap, item_y, item_box_width, item_box_height);
        }
    } else {
        gfx_HorizLine(item_x + item_box_width + item_gap + 8, item_y + (item_box_height / 2), item_box_width - 16);
    }

    gfx_Rectangle(offset_x + 5, offset_y + 76, box_width - 10, 30);
    switch (focused_slot) {
        case 0:
            gfx_PrintStringXY(bought[0] ? "Bought" : "Upgrade", offset_x + 12, offset_y + 84);
            gfx_PrintStringXY(hand_type_label(shape_hand_map[planet_index_1]), offset_x + 70, offset_y + 84);
            if (!bought[0]) {
                gfx_PrintStringXY("$3", offset_x + box_width - 26, offset_y + 84);
            }
            break;
        case 1:
            gfx_PrintStringXY(bought[1] ? "Bought" : "Upgrade", offset_x + 12, offset_y + 84);
            gfx_PrintStringXY(hand_type_label(shape_hand_map[planet_index_2]), offset_x + 70, offset_y + 84);
            if (!bought[1]) {
                gfx_PrintStringXY("$3", offset_x + box_width - 26, offset_y + 84);
            }
            break;
        case 2:
            gfx_PrintStringXY("Leave shop", offset_x + 12, offset_y + 84);
            break;
        case 3:
            gfx_PrintStringXY("Reroll offers", offset_x + 12, offset_y + 84);
            gfx_PrintStringXY("$5", offset_x + box_width - 26, offset_y + 84);
            break;
    }
}

void draw_pre_shop(int score, int hands_left, int discards_left, int money, HandValue hand_value, int current_blind, bool focused_continue) {
    gfx_FillScreen(255);
    gfx_SetTextScale(1, 2);
    display_game_stats(score, 0, 0, hands_left, discards_left, money, hand_value);

    int offset_x = 90;
    int offset_y = 115;
    int box_width = GFX_LCD_WIDTH - offset_x - 50;

    gfx_Rectangle(offset_x, offset_y, box_width, GFX_LCD_HEIGHT - offset_y);
    gfx_Rectangle(offset_x + 5, offset_y + 5, box_width - 10, (GFX_LCD_HEIGHT - offset_y) / 4);
    gfx_PrintStringXY("Cash Out", offset_x + 10, offset_y + 10);
    gfx_SetTextXY(offset_x + 10, offset_y + 26);
    gfx_PrintChar('$');
    gfx_PrintInt((current_blind == 0 ? 5 : current_blind == 1 ? 3 : 4) + hands_left + (money > 25 ? 5 : money / 5), 1);
    if (focused_continue) {
        draw_focus_box(offset_x + 5, offset_y + 5, box_width - 10, (GFX_LCD_HEIGHT - offset_y) / 4);
    }
}

void display_hand(const Hand *hand) {
    for (int i = 0; i < hand->hand_size; i++) {
        int offset_y = 0;
        int offset_x = 74;

        if (hand->hand[i].is_selected || hand->hand[i].to_play) {
            offset_y -= 16;
        }

        print_card(hand->hand[i], offset_x + (32 * i), offset_y + 200);
    }
}

void display_hand_ui(const Hand *hand, UiFocusRegion focus_region, int focused_index, int held_index, UiMode mode) {
    (void)focus_region;
    (void)focused_index;

    for (int i = 0; i < hand->hand_size; i++) {
        int offset_y = 0;
        int offset_x = 74;
        int x;
        int y;

        if (hand->hand[i].is_selected || hand->hand[i].to_play) {
            offset_y -= 16;
        }

        x = offset_x + (32 * i);
        y = offset_y + 200;
        print_card(hand->hand[i], x, y);

        if (mode == UI_MODE_REORDER && held_index == i) {
            gfx_SetTextXY(x - 2, y - 14);
            gfx_PrintString("[]");
        }
    }
}

void display_playing_hand(const Card *cards, int count, const Card *scoring_cards, int pos) {
    int offset_x = 120;
    int offset_y = 100;

    for (int i = 0; i < count; i++) {
        print_playing_card(cards[i], offset_x + (i * 32), offset_y);
        if (pos >= 0 && card_equal(&cards[i], &scoring_cards[pos])) {
            print_playing_value(scoring_cards[pos].value, offset_x + (i * 32), offset_y - 24);
        }
    }
}

void draw_gameplay_actions(UiFocusRegion focus_region, int focused_action, UiMode mode) {
    static const char *labels[ACTION_COUNT] = { "Play", "Discard", "Reorder" };
    int start_x = 86;
    int y = 216;
    int width = 48;
    int height = 18;
    int gap = 8;

    gfx_SetTextScale(1, 1);
    for (int i = 0; i < ACTION_COUNT; i++) {
        int x = start_x + (i * (width + gap));

        gfx_Rectangle(x, y, width, height);
        if (focus_region == UI_FOCUS_ACTIONS && focused_action == i) {
            draw_focus_box(x, y, width, height);
        }
        if (mode == UI_MODE_REORDER && i == ACTION_REORDER) {
            gfx_Rectangle(x + 2, y + 2, width - 4, height - 4);
        }
        gfx_PrintStringXY(labels[i], x + 6, y + 6);
    }
}

void draw_gameplay_footer(UiFocusRegion focus_region, UiMode mode, int held_index) {
    (void)focus_region;
    (void)mode;
    (void)held_index;
}

void draw_blind_footer(void) {
}

void draw_shop_footer(bool in_pre_shop) {
    (void)in_pre_shop;
}
