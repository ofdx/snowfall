GCC_ARGS=--std=c++11 -s -Ibuild/

all: build build/encoder build/picogamo win

clean:
	rm -rf build

run: all
	build/picogamo

runwin: win
	wine build/picogamo.exe

build:
	mkdir build

build/encoder: src/encoder.c src/base64.h
	gcc -o build/encoder src/encoder.c

# Build the game for 64-bit Windows
win: build/picogamo.exe

build/picogamo.exe: build/picogamo
	x86_64-w64-mingw32-g++ $(GCC_ARGS) -o build/picogamo.exe src/main.cc -Lx86_64/lib -Ix86_64/include -lmingw32 -lSDL2main -lSDL2 -mwindows  -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++
	cp x86_64/bin/SDL2.dll build/

# Build the game for 64-bit Linux
build/picogamo: src/main.cc src/loader.h
	g++ $(GCC_ARGS) -lSDL2 -o build/picogamo src/main.cc
