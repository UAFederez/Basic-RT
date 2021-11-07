CC  = g++
SRC = main.cpp
CF  = -Wextra -Wpedantic -O3 -g
LF  = -lsfml-graphics -lsfml-window -lsfml-system

ID  = -IC:/sfml_libs/include -DSFML_STATIC
LD  = -LC:/sfml_libs/lib

all: $(SRC)
	$(CC) $^ $(ID) $(LD) $(CF) $(LF)
