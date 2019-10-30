GCC_ARGS=--std=c++11 -s -Ibuild/

all: build build/assetblob build/picogamo win

clean:
	rm -rf build

run: all
	build/picogamo

runwin: all
	wine build/picogamo.exe

build:
	mkdir build


# Base64 encode asset files into a single file.
blob: build build/assetblob
	
build/assetblob: assets build/encoder
	util/encode

build/encoder: src/encoder.c src/base64.h
	gcc -o build/encoder src/encoder.c


# Build the game for 64-bit Windows
win: build/picogamo.exe

build/picogamo.exe: build/picogamo
	x86_64-w64-mingw32-g++ $(GCC_ARGS) -static -o build/picogamo.exe src/main.cc -Lx86_64/lib -Lx86_64_image/lib -Ix86_64/include/SDL2 -Ix86_64_image/include/SDL2 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -mwindows  -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++


# Build the game for 64-bit Linux
build/picogamo: src/main.cc src/loader.h src/base64.h build/assetblob src/snow.h src/particle.h
	g++ $(GCC_ARGS) -no-pie -I/usr/include/SDL2 -lSDL2 -lSDL2_image -o build/picogamo src/main.cc
