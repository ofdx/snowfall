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
	x86_64-w64-mingw32-g++ $(GCC_ARGS) -o build/picogamo.exe src/main.cc -Lx86_64/lib -Ix86_64/include -lmingw32 -lSDL2main -lSDL2 -mwindows  -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++
	cp x86_64/bin/SDL2.dll build/


# Build the game for 64-bit Linux
build/picogamo: src/main.cc src/loader.h src/base64.h build/assetblob
	g++ $(GCC_ARGS) -lSDL2 -o build/picogamo src/main.cc
