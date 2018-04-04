CC=gcc
CXX=g++
COMMON_FLAGS=-g -O0 -W -Wall -Wextra -pedantic -pedantic-errors
CFLAGS=$(COMMON_FLAGS) -Wno-missing-field-initializers -Wno-missing-braces -std=c89 -ansi
CXXFLAGS=$(COMMON_FLAGS) -std=c++11
LDFLAGS=-g
DEPS=libdo.h
OBJC=tests.o libdo.o
OBJCXX=testscpp.o libdo.o

RM=rm -f
RMDIR=rm -rf

all: tests testscpp

.PHONY: tests testscpp

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

tests: $(OBJC)
	$(CC) $(LDFLAGS) -o $@ $^

testscpp: $(OBJCXX)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(OBJC) $(OBJCXX) tests testscpp
