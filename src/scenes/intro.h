class IntroSplashScene : public Scene {
private:
	ParticleEffect *snow_left, *snow_right;

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
	}

	~IntroSplashScene(){
		delete snow_left;
		delete snow_right;
	}
};
