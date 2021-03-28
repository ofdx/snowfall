GCC_ARGS=-Ofast -Wall --std=c++11 -s -Ibuild/
MINGW=x86_64-w64-mingw32-g++

all: build build/assetblob build/neigetombe win

clean:
	@echo "Removing build output directory..."
	@rm -rf build

run: all
	@build/neigetombe

runwin: all
	@wine build/neigetombe.exe

build:
	@mkdir build


# Base64 encode asset files into a single file.
blob: build build/assetblob
	
build/assetblob: assets build/encoder
	@echo "Encoding and combining assets..."
	@util/encode

build/encoder: src/encoder.c src/base64.h
	@echo "Building base64 encode utility..."
	@gcc -o build/encoder src/encoder.c


# Build the game for 64-bit Windows
win: build/neigetombe.exe

build/neigetombe.exe: build/neigetombe
	@echo "Building for Windows..."
	@if [ -n "`which "$(MINGW)"`" ]; then \
		$(MINGW) $(GCC_ARGS) -static -o build/neigetombe.exe src/main.cc -Lx86_64/lib -Lx86_64_mixer/lib -Ix86_64/include -Ix86_64/include/SDL2 -Ix86_64_mixer/include -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -mwindows  -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++; \
		if [ -e build/neigetombe.exe ]; then chmod a-x build/neigetombe.exe; fi; \
	else \
		touch build/neigetombe.exe; \
		echo "  - No Windows build environment available."; \
	fi


# Build the game for 64-bit Linux
build/neigetombe: src/main.cc build/assetblob src/*.h src/scenes/*
	@echo "Building for Linux..."
	@g++ $(GCC_ARGS) -no-pie -I/usr/include -o build/neigetombe src/main.cc -lSDL2main -lSDL2 -lSDL2_mixer
