class IntroSplashScene : public Scene {
private:
	ParticleEffect *snow_left, *snow_right;
	PicoText *deadbeef;

public:
	IntroSplashScene(Scene::Controller *ctrl) : Scene(ctrl) {
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

		drawables.push_back(snow_left);
		drawables.push_back(snow_right);

		deadbeef = new PicoText(rend, (SDL_Rect){
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT
		}, "deadbeef");
		deadbeef->set_font("fonts/24x28.bmp", 24, 28);
		drawables.push_back(deadbeef);
	}

	~IntroSplashScene(){
		delete snow_left;
		delete snow_right;
		delete deadbeef;
	}
};
