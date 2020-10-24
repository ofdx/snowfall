class IntroSplashScene : public Scene {
	int ticks_total = 0;

public:
	IntroSplashScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "krakcircle.bmp");

		// Immediately play stinger sound.
		try {
			Mix_PlayChannel(-1, FileLoader::get("stinger.wav")->sound(), 0);
		} catch(...){
			cout << "Failed to find file: stinger.wav" << endl;
		}

	}

	void draw(int ticks){
		Scene::draw(ticks);
		ticks_total += ticks;

		// Wait about three seconds.
		if(ticks_total > 2950)
			ctrl->set_scene(Scene::create(ctrl, "forest"));
	}
};
