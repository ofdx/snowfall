class IntroSplashScene : public Scene {
private:
	SDL_Texture *bg;

	ParticleEffect *snow_left, *snow_right;

public:
	IntroSplashScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "krakcircle.bmp");

		snow_left = new SnowEffect(
			rend,
			(SDL_Rect){
				0,0,
				SCREEN_WIDTH / 2, SCREEN_HEIGHT
			},
			10, 50, 40,
			90, 10,
			30
		);

		snow_right = new SnowEffect(
			rend,
			(SDL_Rect){
				SCREEN_WIDTH / 2, 0,
				SCREEN_WIDTH / 2, SCREEN_HEIGHT
			},
			10, 50, 40,
			-90, 10,
			30
		);

	}

	~IntroSplashScene(){
		SDL_DestroyTexture(bg);

		delete snow_left;
		delete snow_right;
	}

	void draw(int ticks){
		SDL_RenderCopy(rend, bg, NULL, NULL);

		snow_left->update(ticks);
		snow_right->update(ticks);
	}
};
