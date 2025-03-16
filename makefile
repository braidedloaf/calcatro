# Makefile for TI-84 Plus CE C program

NAME = BALATRO
SOURCES = main.c

COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz


include $(shell cedev-config --makefile)