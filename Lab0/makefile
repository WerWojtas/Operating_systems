CXX=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -pedantic -g

SRCS=$(wildcard *.c)
EXECS=$(SRCS:.c=)

.PHONY: all countdown clean

all: $(EXECS)

countdown: countdown.c
	$(CXX) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXECS)