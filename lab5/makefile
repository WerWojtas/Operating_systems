CXX=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

.PHONY: all

all: integral calculate receive

integral:
	$(CXX) integral.c -o integral

calculate:
	$(CXX) calculate.c -o calculate

receive:
	$(CXX) receive.c -o receive

clean:
	-rm integral receive calculate
