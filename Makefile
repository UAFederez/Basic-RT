CC  = g++
SRC = main.cpp graphics/Scene.cpp graphics/Triangle.cpp graphics/Plane.cpp graphics/Rectangle3D.cpp graphics/Sphere.cpp graphics/Camera.cpp
CF  = -Wextra -Wpedantic -O3 -g
LF  = -lsfml-graphics -lsfml-window -lsfml-system

ID  = -IC:/sfml_libs/include
LD  = -LC:/sfml_libs/lib

all: $(SRC)
	$(CC) $^ $(CF) $(ID) $(LD) $(LF) 
