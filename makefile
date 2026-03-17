NAME = CALCATRO
SOURCES = main.c hand_eval.c deck.c ui.c gameplay.c
ICON = icon.png

COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

include $(shell cedev-config --makefile)
