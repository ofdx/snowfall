#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <map>
#include <string>
#include <list>
#include <cmath>
#include <vector>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 150

#define PI 3.14159265359

using namespace std;
int render_scale = 5;

#include "loader.h"
#include "utility.h"
#include "drawable.h"
#include "clickable.h"
#include "cardpanel.h"

#include "text.h"
#include "scene.h"
#include "scene3d.h"
#include "particle.h"
#include "button.h"
#include "card.h"

// Particle effects
#include "snow.h"

// Scenes
#include "scenes/intro.h"
#include "scenes/living.h"
#include "scenes/jeep.h"
#include "scenes/garage.h"
#include "scenes/forest.h"
#include "scenes/cards.h"
#include "scenes/test3d.h"

int main(int argc, char **argv){
#include "assetblob"

	SDL_Event event;

    if(SDL_Init(SDL_INIT_VIDEO)){
		cout << "Failed to init SDL: " << SDL_GetError() << endl;
		return -1;
	}
	SDL_ShowCursor(SDL_DISABLE);

	// Enable audio
	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)){
		cout << "Failed to initialize audio." << endl;
		return -2;
	}

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

	SDL_Texture *mouse_tx_1 = textureFromBmp(rend, "mouse/cursor.bmp", true);
	SDL_Texture *mouse_tx_2 = textureFromBmp(rend, "mouse/cursor2.bmp", true);
	SDL_Texture *mouse_tx = mouse_tx_1;

	SDL_Rect fillRect = { SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 32, 32 };
	SDL_Rect outlRect = { SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 20, 40, 40 };
	SDL_Rect holder = {};

	double offset_x = 0.0;
	double offset_y = 0.0;

	map<int, bool> keys;

	// Create pointers to scene constructors by name.
	Scene::reg("intro", scene_create<IntroSplashScene>);
	Scene::reg("living", scene_create<LivingRoomScene>);
	Scene::reg("jeep", scene_create<JeepScene>);
	Scene::reg("garage", scene_create<GarageScene>);
	Scene::reg("forest", scene_create<ForestScene>);
	Scene::reg("cards", scene_create<CardsScene>);
	Scene::reg("test3d", scene_create<TestScene3D>);

	Scene::Controller *ctrl = new Scene::Controller(rend);
	ctrl->set_scene(Scene::create(ctrl, "intro"));

	// Intro splash
	/*{
		int ticks_start = SDL_GetTicks();
		int ticks_prev = ticks_start;
		bool skip = false;

		while(!skip){
			int ticks_now = SDL_GetTicks();
			int ticks = (ticks_now - ticks_prev);
			ticks_prev = ticks_now;

			ctrl->draw(ticks);

			SDL_RenderPresent(rend);

			if((ticks_now - ticks_start) > 3000){
				while(SDL_PollEvent(&event)){
					if(event.type == SDL_KEYDOWN){
						skip = true;
						break;
					}
				}
			}

			SDL_Delay(1000 / 60);
		}
	}*/

	// Simple rectangle representing the mouse cursor.
	SDL_Rect mouse_cursor = { SCREEN_WIDTH, SCREEN_HEIGHT, 14, 14 };

	// Load the first scene.
	ctrl->set_scene(Scene::create(ctrl, "forest"));

	bool render_reticule = false;
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

				case SDL_MOUSEMOTION:
					// Move cursor to point position.
					mouse_cursor.x = event.motion.x / render_scale;
					mouse_cursor.y = event.motion.y / render_scale;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					ctrl->check_mouse(event);
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
		if(keys[SDLK_q] && !(dynamic_cast<LivingRoomScene*>(ctrl->scene)))
			ctrl->set_scene(Scene::create(ctrl, "living"));

		if(keys[SDLK_e] && !(dynamic_cast<GarageScene*>(ctrl->scene)))
			ctrl->set_scene(Scene::create(ctrl, "garage"));

		if(keys[SDLK_1] && !(dynamic_cast<JeepScene*>(ctrl->scene)))
			ctrl->set_scene(Scene::create(ctrl, "jeep"));

		// Test multiple mouse cursors.
		if(keys[SDLK_2])
			mouse_tx = mouse_tx_2;
		if(keys[SDLK_3])
			mouse_tx = mouse_tx_1;

		// Toggle targeting reticule visibility.
		if(keys[SDLK_r])
			render_reticule = !render_reticule;

		// Draw the current scene.
		ctrl->draw(ticks);

		// Draw the targeting reticule.
		if(render_reticule){
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

		// Draw the mouse cursor
		SDL_RenderCopy(rend, mouse_tx, NULL, &mouse_cursor);

		// Flip the display buffer.
		SDL_RenderPresent(rend);

		// Delay to limit to approximately 60 fps.
		SDL_Delay(1000 / 60);

		// Update tick counter.
		ticks_last = ticks_now;
	}

quit:
	// Clean up and close SDL library.
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}
