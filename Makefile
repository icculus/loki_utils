#
# $Id$
#
# Makefile for the Loki Utilities Library
#

# This should be moved in a global configuration file
DEBUG = true
CC = gcc
CPP = g++
AR = ar rcs


CFLAGS += -Wall
ifeq ($(DEBUG),true)
CFLAGS += -g
else
CFLAGS += -O2
endif
CFLAGS += -D_REENTRANT -D_SDL_STATIC_LIB -I../SDL/include

CXXFLAGS = $(CFLAGS)
.SUFFIXES: .c .cpp

OBJS = loki_config.o loki_network.o loki_paths.o loki_signals.o loki_utils.o

TARGET = libloki.a

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CPP) $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	$(AR) $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
