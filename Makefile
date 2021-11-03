CC     = g++
SRC    = main.cpp
CFLAGS = -static-libgcc -static-libstdc++ -Wpedantic -Wall -Wextra -g -O3
all: $(SRC)
	$(CC) $^ $(CFLAGS)
