class JeepScene : public Scene {
private:
	ParticleEffect *snow;

public:
	JeepScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "jeep.bmp");

		snow = new SnowEffect(
			rend,
			(SDL_Rect){ 
				20, 20,
				SCREEN_WIDTH - 40, SCREEN_HEIGHT - 40 
			},
			10, 50, 40,
			-90, 10,
			150
		);

		drawables.push_back(snow);
	}

	~JeepScene(){
		delete snow;
	}
};
