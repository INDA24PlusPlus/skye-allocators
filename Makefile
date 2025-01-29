CFLAGS = -Wall -Wextra -g -O0
CLIBS=-lc 
CC = g++
mallocator: main.o
	$(CC) $(CFLAGS) $? -o $@ $(CLIBS)
main.o: main.cpp
	$(CC) $(CFLAGS) -c $? -o $@ $(CLIBS)

