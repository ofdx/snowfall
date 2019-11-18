class ForestScene : public Scene {
	SDL_Texture *forest;
	SDL_Rect pan;

	float pan_vertical = -15.0f;
	bool panning = true;

	class PlayButton : public Button {
		Scene::Controller *ctrl;

	public:
		PlayButton(Scene::Controller *ctrl, SDL_Rect click_region, string text) : Button(ctrl->renderer(), click_region, text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->set_scene(Scene::create(ctrl, "living"));
		}
	} *playButton;

	class QuitButton : public Button {
	public:
		QuitButton(SDL_Renderer *rend, SDL_Rect click_region, string text) : Button(rend, click_region, text) {}

		void action(){
			exit(0);
		}
	} *quitButton;

	Mix_Music *music;

public:
	ForestScene(Scene::Controller *ctrl) : Scene(ctrl) {
		forest = textureFromBmp(rend, "pict0007.bmp");
		pan = (SDL_Rect){
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT
		};


		SDL_Rect menu_button = {
			3, SCREEN_HEIGHT - 18,
			120, 15
		};
		quitButton = new QuitButton(rend, menu_button, "Quit");

		menu_button.y -= (menu_button.h + 3);
		playButton = new PlayButton(ctrl, menu_button, "Begin Adventure...");

		{
			FileLoader *fl = FileLoader::get("t2.wav");

			if(fl){
				music = fl->music();
				Mix_PlayMusic(music, -1);
			} else {
				cout << "Failed to find file: t2.wav" << endl;
			}
		}
	}

	~ForestScene(){
		if(Mix_PlayingMusic())
			Mix_HaltMusic();

		SDL_DestroyTexture(forest);

		delete playButton;
	}

	void draw(int ticks){
		if(panning){
			pan_vertical += (ticks / 200.0f);

			if(pan_vertical > 0){
				pan_vertical = 0;
				panning = false;

				// Start drawing menu items
				drawables.push_back(playButton);
				clickables.push_back(playButton);

				drawables.push_back(quitButton);
				clickables.push_back(quitButton);
			}

			pan.y = (384 - SCREEN_HEIGHT) - (pan_vertical * pan_vertical);
		}

		SDL_RenderCopy(rend, forest, &pan, NULL);

		Scene::draw(ticks);
	}
};
