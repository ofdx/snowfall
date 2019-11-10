GCC_ARGS=-Wall --std=c++11 -s -Ibuild/
MINGW=x86_64-w64-mingw32-g++

all: build build/assetblob build/picogamo win

clean:
	@echo "Removing build output directory..."
	@rm -rf build

run: all
	@build/picogamo

runwin: all
	@wine build/picogamo.exe

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
win: build/picogamo.exe

build/picogamo.exe: build/picogamo
	@echo "Building for Windows..."
	@if [ -n "`which "$(MINGW)"`" ]; then \
		$(MINGW) $(GCC_ARGS) -static -o build/picogamo.exe src/main.cc -Lx86_64/lib -Lx86_64_mixer/lib -Ix86_64/include -Ix86_64/include/SDL2 -Ix86_64_mixer/include -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -mwindows  -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++; \
		if [ -e build/picogamo.exe ]; then chmod a-x build/picogamo.exe; fi; \
	else \
		touch build/picogamo.exe; \
		echo "  - No Windows build environment available."; \
	fi


# Build the game for 64-bit Linux
build/picogamo: src/main.cc build/assetblob src/*.h src/scenes/*
	@echo "Building for Linux..."
	@g++ $(GCC_ARGS) -no-pie -I/usr/include -lSDL2 -lSDL2_mixer -o build/picogamo src/main.cc
