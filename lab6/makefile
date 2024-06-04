CXX=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

.PHONY: all

all: server client

server:
	$(CXX) server.c -o server

client:
	$(CXX) client.c -o client

clean:
	-rm server client