#
# $Id$
#
# Makefile for the Loki Utilities Library
#

# This should be moved in a global configuration file
DEBUG = true
CC = gcc
CXX = g++
AR = ar rcs


CFLAGS += -Wall
INCLUDES =  -I/usr/local/include/SDL
CFLAGS += $(INCLUDES)
ifeq ($(DEBUG),true)
CFLAGS += -g -O
else
CFLAGS += -O2
endif
ifneq ($(sdl_utils), false)
# Comment the following line if using dynamic SDL
# CFLAGS += -D_SDL_STATIC_LIB
CFLAGS += -D_REENTRANT
endif
ifeq ($(windowed_only), true)
CFLAGS += -DWINDOWED_ONLY
endif

CXXFLAGS = $(CFLAGS)
.SUFFIXES: .c .cpp

CSRC	= loki_config.c loki_network.c loki_paths.c loki_signals.c loki_utils.c
CPPSRC	= 
ifneq ($(sdl_utils), false)
CSRC	+= sdl_pcx.c
CPPSRC	+= sdl_utils.cpp
endif

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o) 

TARGET = libloki.a

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	$(AR) $(TARGET) $(OBJS)

testargs: $(TARGET) testargs.c
	$(CC) $(CFLAGS) -o testargs testargs.c -L. -lloki

clean:
	rm -f $(OBJS) $(TARGET)

dep: depend

depend:
	$(CXX) -MM $(INCLUDES) $(CSRC) $(CPPSRC) > .depend

ifeq ($(wildcard .depend),.depend)
include .depend
endif
