CC   = g++
SRC  = main.cpp \
	   graphics/Scene.cpp \
	   graphics/Triangle.cpp \
	   graphics/Plane.cpp \
	   graphics/Rectangle3D.cpp \
	   graphics/Sphere.cpp \
	   graphics/Camera.cpp \
	   util/BitmapImage.cpp \
	   util/Threading.cpp

# TODO: autogenerate if not exisiting
BDIR = build
CF   = -Wextra -Wpedantic -O3 -g -DSFML_STAIC -static-libstdc++ -static-libgcc
LF   = -lsfml-graphics -lsfml-window -lsfml-system

ID   = -IC:/sfml_libs/include 
LD   = -LC:/sfml_libs/lib

EXT  = exe

Raytracer.$(EXT): util/General.h \
	   		   	  $(BDIR)/main.o \
	   		   	  $(BDIR)/Scene.o \
	   		   	  $(BDIR)/Triangle.o \
	   		   	  $(BDIR)/Plane.o \
	   		   	  $(BDIR)/Rectangle3D.o \
	   		   	  $(BDIR)/Sphere.o \
	   		   	  $(BDIR)/Camera.o \
	   		   	  $(BDIR)/BitmapImage.o \
	   		   	  $(BDIR)/Threading.o
	@$(CC) $^ -o $@ $(CF) $(ID) $(LD) $(LF)
	@echo [Linking  ] Making executable

$(BDIR)/main.o: main.cpp \
				graphics/Material.h \
				graphics/Camera.h \
				graphics/Sphere.h \
				graphics/Triangle.h \
				graphics/Rectangle3D.h \
				graphics/Plane.h \
				graphics/Scene.h \
				util/BitmapImage.h \
				util/Threading.h \
				util/General.h \
	   			math/Vector.h
	@$(CC) -c $< $(ID) -o $@
	@echo [Compiling] $<

$(BDIR)/%.o: graphics/%.cpp graphics/%.h
	@$(CC) -c $< $(ID) -o $@
	@echo [Compiling] $<

$(BDIR)/%.o: util/%.cpp util/%.h
	@$(CC) -c $< $(ID) -o $@
	@echo [Compiling] $<

clean:
	del /S *o Raytracer.exe Raytracer.out
