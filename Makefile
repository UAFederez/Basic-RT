CC   = g++
SRC  = main.cpp \
	   graphics/Scene.cpp \
	   graphics/Triangle.cpp \
	   graphics/Plane.cpp \
	   graphics/Rectangle3D.cpp \
	   graphics/Mesh.cpp \
	   graphics/Sphere.cpp \
	   graphics/Material.cpp \
	   graphics/Camera.cpp \
	   util/BitmapImage.cpp \
	   util/Threading.cpp 

# TODO: autogenerate if not exisiting
BDIR = build
CF   = -Wall -Wextra -Wpedantic -g -O3 -DSFML_STATIC
LF   = -lsfml-system -lsfml-graphics -lsfml-window
ID   = -IC:/sfml_libs/include 
LD   = -LC:/sfml_libs/lib


ifeq ($(OS), Windows_NT)
	EXT  = exe
	DEL  = del
	DELF = /S
else
	EXT  = out
	DEL  = rm
	DELF = -rf
endif

Raytracer.$(EXT): util/General.h \
	   		   	  $(BDIR)/main.o \
	   		   	  $(BDIR)/Scene.o \
	   		   	  $(BDIR)/Triangle.o \
	   		   	  $(BDIR)/Plane.o \
	   		   	  $(BDIR)/Rectangle3D.o \
	   		   	  $(BDIR)/Sphere.o \
	   		   	  $(BDIR)/Material.o \
	   		   	  $(BDIR)/Threading.o \
	   		   	  $(BDIR)/Mesh.o \
	   		   	  $(BDIR)/Camera.o \
	   		   	  $(BDIR)/BitmapImage.o
	@$(CC) -o $@ $^ $(CF) $(ID) $(LD) $(LF)
	@echo [Linking..] Making executable

$(BDIR)/main.o: main.cpp \
				graphics/Material.h \
				graphics/Camera.h \
				graphics/Sphere.h \
				graphics/Triangle.h \
				graphics/Rectangle3D.h \
				graphics/Plane.h \
				graphics/Scene.h \
				graphics/Mesh.h \
				util/Threading.h \
				util/BitmapImage.h \
				util/General.h \
	   			math/Vector.h
	@$(CC) -c $< $(CF) $(ID) -o $@
	@echo [Compiling] $<

$(BDIR)/%.o: graphics/%.cpp graphics/%.h
	@$(CC) -c $< $(CF) $(ID) -o $@
	@echo [Compiling] $<

$(BDIR)/%.o: util/%.cpp util/%.h
	@$(CC) -c $< $(CF) $(ID) -o $@
	@echo [Compiling] $<

clean:
ifeq ($(OS), Windows_NT)
	del /S *o *gch Raytracer.exe Raytracer.out
else
	$(DEL) $(DELF) $(BDIR)/*.o Raytracer.exe Raytracer.out
endif

