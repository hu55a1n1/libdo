CC = gcc
CFLAGS = -O0 -g -W -Wall -Wextra -Wno-missing-field-initializers -Wno-missing-braces -std=c89 -ansi -pedantic -pedantic-errors

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra -std=c++11

all: tests testscpp

.PHONY: tests testscpp
tests: tests.c libdo.c
	$(CC) $(CFLAGS) -o $@ tests.c libdo.c

testscpp: testscpp.cpp libdo.c
	$(CPPC) $(CPPFLAGS) -o $@ testscpp.cpp libdo.c

clean:
	rm -f tests testscpp *.o
	rm -rf *.dSYM
