CXX=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

.PHONY: all

all: signal sender catcher

signal:
	$(CXX) signal.c -o signal

sender:
	$(CXX) sender.c -o sender

catcher:
	$(CXX) catcher.c -o catcher

clean:
	-rm signal sender catcher
