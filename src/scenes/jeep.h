class JeepScene : public Scene {
private:
	SDL_Texture *bg;

	SDL_Rect snowspace;
	ParticleEffect *snow;

public:
	JeepScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "jeep.bmp");

		snowspace = { 20,20, SCREEN_WIDTH-40,SCREEN_HEIGHT-40 };
		snow = new SnowEffect(
			rend,
			snowspace,
			10, 50, 40,
			-90, 10,
			150
		);
	}

	~JeepScene(){
		SDL_DestroyTexture(bg);

		delete snow;
	}

	void draw(int ticks){
		// Draw the background image.
		SDL_RenderCopy(rend, bg, NULL, NULL);

		// FIXME debug
		// Draw bounding box around snow flakes.
		SDL_SetRenderDrawColor(rend, 0x00, 0xFF, 0x00, 0xA0);
		SDL_RenderDrawRect(rend, &snowspace);

		// Draw snow flakes.
		snow->update(ticks);
	}
};
