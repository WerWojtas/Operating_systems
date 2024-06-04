CXX=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -g

.PHONY: all

all: proces proces2

proces:
	$(CXX) $(CFLAGS) proces.c -o proces

proces2:
	$(CXX) $(CFLAGS) proces2.c -o proces2

clean:
	-rm proces proces2
