NAME = CALCATRO
SOURCES = main.c
ICON = icon.png

COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

include $(shell cedev-config --makefile)