CC = gcc
CXX = g++
CFLAGS= -Wall -O3
CXXFLAGS= -Wall -O0 -g --std=gnu++17

default: all
all:
	$(MAKE) clean
	$(MAKE) stenomesh

test: check
check: all
	./test/all.sh

stenomesh: src/tinyply.o src/stenomesh.o
	$(CXX) $(CXXFLAGS) -o stenomesh src/stenomesh.o src/tinyply.o

.PHONY: clean
clean:
	rm -f stenomesh src/*.o
