#
# Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
# All rights reserved.
#

OBJS:=$(patsubst %.cpp, %.o, $(wildcard *.cpp))
OBJS+=$(patsubst %.c, %.o, $(wildcard *.c))
CFLAGS:=-g -DTASK_DEBUG -Werror -Wall

.PHONY: all
all: main

main: $(OBJS)
	@echo "Linking... $@"
	@$(CXX) $(CFLAGS) $^ -o $@

%.o: %.cpp
	  @echo "[CXX] $@"
	  @$(CXX) $(CFLAGS) $(INC) -c $< -o $@

%.o: %.c
	 @echo "[CXX] $@"
	 @$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *.o main
