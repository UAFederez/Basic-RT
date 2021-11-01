all: main.cpp
	g++ $^ -Wall -Wextra -Wpedantic -g -lpthread -O3
