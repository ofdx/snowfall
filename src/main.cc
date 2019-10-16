#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <string>
#include <list>
#include <cmath>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 150

using namespace std;

#include "loader.h"
#include "particle.h"
#include "snow.h"

SDL_Texture *textureFromBmp(SDL_Renderer *rend, const char *fn){
	FileLoader *fl = FileLoader::get(fn);
	if(!fl)
		return NULL;

	SDL_Surface *sf = fl->surface();
	SDL_Texture *tx = SDL_CreateTextureFromSurface(rend, sf);
	SDL_FreeSurface(sf);

	return tx;
}

void rectSum(SDL_Rect &holder, SDL_Rect a, SDL_Rect b){
	holder.x = (a.x + b.x);
	holder.y = (a.y + b.y);
	holder.w = (a.w + b.w);
	holder.h = (a.h + b.h);
}
void rectFloatOffset(SDL_Rect &holder, SDL_Rect rect, double x, double y){
	holder.x = (int)(rect.x + x);
	holder.y = (int)(rect.y + y);
	holder.w = rect.w;
	holder.h = rect.h;
}


int main(int argc, char **argv){
#include "assetblob"

	SDL_Event event;

    if(SDL_Init(SDL_INIT_VIDEO)){
		cout << "Failed to init SDL: " << SDL_GetError() << endl;
		return -1;
	}

	int render_scale = 5;

	// Automatically set default value based on desktop resolution.
	{
		SDL_DisplayMode mode;
		if(!SDL_GetDesktopDisplayMode(0, &mode)){
			int width = mode.w / SCREEN_WIDTH;
			int height = mode.h / SCREEN_HEIGHT;

			render_scale = ((width > height) ? height : width) - 1;
		}
	}

	SDL_Window *win = SDL_CreateWindow("picogamo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * render_scale, SCREEN_HEIGHT * render_scale, SDL_WINDOW_SHOWN);

	// Create a renderer for the window we'll draw everything to.
	SDL_Renderer *rend = SDL_CreateRenderer(win, -1, 0);
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_RenderSetScale(rend, render_scale, render_scale);

	SDL_Texture *tx1 = textureFromBmp(rend, "./living/1.bmp");
	SDL_Texture *tx2 = textureFromBmp(rend, "./spacebun.bmp");
	SDL_Texture *tx3 = textureFromBmp(rend, "./jeep.bmp");
	SDL_Texture *texture = tx3;

	SDL_Rect fillRect = { SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 32, 32 };
	SDL_Rect outlRect = { SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 20, 40, 40 };
	SDL_Rect holder = {};

	double offset_x = 0.0;
	double offset_y = 0.0;

	map<int, bool> keys;

	// Intro splash
	{
		SDL_Texture *tx_help = textureFromBmp(rend, "./krakcircle.bmp");
		SDL_RenderCopy(rend, tx_help, NULL, NULL);
		SDL_RenderPresent(rend);

		SDL_Delay(3000);
	}

	// FIXME debug - show help at startup
	{
		SDL_Texture *tx_help = textureFromBmp(rend, "./help.bmp");
		SDL_RenderCopy(rend, tx_help, NULL, NULL);
		SDL_RenderPresent(rend);

		// Eat up enqueued events.
		while(SDL_PollEvent(&event));

		// Wait for a keypress.
		while(1){
			SDL_WaitEvent(&event);

			if(event.type == SDL_KEYDOWN)
				break;
		}

		cout << FileLoader::get("./desc.txt")->text();
	}

	ParticleEffect *snow = new SnowScene(
		rend,
		(SDL_Rect){ 20,20, SCREEN_WIDTH-40,SCREEN_HEIGHT-40 },
		10, 50, 40,
		-90, 10,
		150
	);

	int ticks_last = SDL_GetTicks();
	while(1){
		int ticks_now = SDL_GetTicks();
		const int ticks = ticks_now - ticks_last;

		// Check for an event without waiting.
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					goto quit;

				// Map out keystates
				case SDL_KEYUP:
					keys[event.key.keysym.sym] = false;
					break;
				case SDL_KEYDOWN:
					keys[event.key.keysym.sym] = true;
					break;
			}
		}

		// Perform draw operation
		int delta_y = 0;
		int delta_x = 0;

		// Handle keys
		if(keys[SDLK_DOWN] || keys[SDLK_s]){
			delta_y += 4;
		}
		if(keys[SDLK_UP] || keys[SDLK_w]){
			delta_y -= 4;
		}
		if(keys[SDLK_LEFT] || keys[SDLK_a]){
			delta_x -= 4;
		}
		if(keys[SDLK_RIGHT] || keys[SDLK_d]){
			delta_x += 4;
		}

		if(!delta_x && !delta_y){
			if((offset_x > 12) || (offset_x < -12))
				delta_x = -(offset_x / 10);
			else if((offset_x > 4) || (offset_x < -4))
				delta_x = -(offset_x / 5);
			else
				offset_x = 0;

			if((offset_y > 12) || (offset_y < -12))
				delta_y = -(offset_y / 10);
			else if((offset_y > 4) || (offset_y < -4))
				delta_y = -(offset_y / 5);
			else
				offset_y = 0;
		} else {
			if((offset_x < -SCREEN_WIDTH) || (offset_x > SCREEN_WIDTH))
				delta_x = 0;

			if((offset_y < -SCREEN_HEIGHT) || (offset_y > SCREEN_HEIGHT))
				delta_y = 0;
		}

		// Hold in place while space is held.
		if(keys[SDLK_SPACE])
			delta_x = delta_y = 0;

		offset_x += delta_x;
		offset_y += delta_y;

		// Swap textures just for fun.
		if(keys[SDLK_q])
			texture = tx1;
		if(keys[SDLK_e])
			texture = tx2;
		if(keys[SDLK_1])
			texture = tx3;


		// Put the background image up.
		SDL_RenderCopy(rend, texture, NULL, NULL);

		if(texture == tx3)
			snow->update(ticks);
		else {
			// Draw a filled rectangle.
			SDL_SetRenderDrawColor(rend, 0xD0, 0x20, 0x00, 0x80);
			rectFloatOffset(holder, fillRect, offset_x * 2.0 / 3.0, offset_y * 2.0 / 3.0);
			SDL_RenderFillRect(rend, &holder);

			// Draw a line rectangle.
			SDL_SetRenderDrawColor(rend, 0xD0, 0x20, 0xD0, 0xD0);
			rectFloatOffset(holder, outlRect, offset_x, offset_y);
			SDL_RenderDrawRect(rend, &holder);

			// Draw points.
			SDL_SetRenderDrawColor(rend, 0x20, 0xD0, 0xD0, 0xFF);
			for(int i = 0; i < SCREEN_WIDTH; i += 4){
				SDL_RenderDrawPoint(rend, i, SCREEN_HEIGHT / 2 - 22);
				SDL_RenderDrawPoint(rend, i, SCREEN_HEIGHT / 2 + 21);
			}
		}

		SDL_RenderPresent(rend);

		// Delay to limit to approximately 60 fps.
		SDL_Delay(1000 / 60);

		// Update tick counter.
		ticks_last = ticks_now;
	}

quit:
	// Clean up and close SDL library.
	SDL_Quit();

	return 0;
}
