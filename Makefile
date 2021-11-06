CC     = g++
SRC    = main.cpp
CFLAGS = -static-libgcc -static-libstdc++ -Wpedantic -Wextra -g -O3 -lsfml-graphics -lsfml-window -lsfml-system
all: $(SRC)
	$(CC) $^ $(CFLAGS)
