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
ARCH = $(shell ./print_arch)
GLMSG = false

INCLUDES += -I/usr/X11R6/include
CFLAGS += -Wall -fsigned-char
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
ifeq ($(BETA), true)
CFLAGS += -DLINUX_BETA
endif
ifeq ($(DEMO), true)
CFLAGS += -DLINUX_DEMO
endif

CXXFLAGS = $(CFLAGS)
.SUFFIXES: .c .cpp

CSRC	= loki_config.c loki_network.c loki_paths.c loki_files.c \
          loki_signals.c loki_qagent.c loki_utils.c loki_inifile.c \
          loki_cpuinfo.c loki_launchurl.c

CPPSRC	= 
ifneq ($(sdl_utils), false)
CSRC	+= sdl_pcx.c loki_2dmessage.c loki_fontdata.c
CPPSRC	+= sdl_utils.cpp
CFLAGS  += $(shell sdl-config --cflags)

ifeq ($(strip $(GLMSG)),true)
CSRC += loki_glmessage.c
else
CFLAGS += -DLOKI_NO_GLMSG
endif

endif

OBJS := $(CSRC:%.c=$(ARCH)/%.o) $(CPPSRC:%.cpp=$(ARCH)/%.o) 

override TARGET = $(ARCH)/libloki.a

$(ARCH)/%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$(ARCH)/%.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(TARGET): $(ARCH) $(OBJS)
	$(AR) $(TARGET) $(OBJS)

$(ARCH):
	mkdir $(ARCH)

testini: testini.c $(TARGET)
	$(CC) $(CFLAGS) -o testini testini.c -L$(ARCH) -lloki

clean:
	rm -f $(ARCH)/*.o
	rm -f $(ARCH)/*.a

distclean: clean
	rm -f $(TARGET)

dep: depend

depend:
	$(CXX) -MM $(CFLAGS) $(CSRC) $(CPPSRC) > .depend

ifeq ($(wildcard .depend),.depend)
include .depend
endif
