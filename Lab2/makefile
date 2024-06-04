CXX=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -g

.PHONY: all

all: copy read

copy:
	$(CXX) $(CFLAGS) copy.c -o copy

read:
	$(CXX) $(CFLAGS) read.c -o read

clean:
	-rm copy read
