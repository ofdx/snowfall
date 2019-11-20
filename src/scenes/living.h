class LivingRoomScene : public Scene {
	class QuitButton : public Button {
		Scene::Controller *ctrl;

	public:
		QuitButton(Scene::Controller *ctrl, SDL_Rect click_region, string text) : Button(ctrl->renderer(), click_region, text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->quit();
		}
	} *quitButton;

	class ChirpButton : public Button {
		Mix_Chunk *atari = NULL;

	public:
		ChirpButton(SDL_Renderer *rend, SDL_Rect click_region, string text) : Button(rend, click_region, text) {
			// Load chirp sound.
			{
				FileLoader *fl = FileLoader::get("atari.wav");

				if(fl){
					atari = fl->sound();
				} else {
					cout << "Failed to find file: atari.wav" << endl;
				}
			}
		}

		// Immediately perform the action (make a chirping sound) when the
		// button is pressed, rather than waiting for a real click (press and
		// release).
		void on_mouse_down(SDL_MouseButtonEvent event){
			// Call the parent function, so that the down state of the button
			// still changes as normal.
			Button::on_mouse_down(event);

			if(atari)
				Mix_PlayChannel(-1, atari, 0);
		}
	} *chirpButton;

	Mix_Music *music;

public:
	LivingRoomScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "living/1.bmp");

		// Load music.
		{
			FileLoader *fl = FileLoader::get("t2.wav");

			if(fl){
				music = fl->music();
				Mix_PlayMusic(music, -1);
			} else {
				cout << "Failed to find file: t2.wav" << endl;
			}
		}

		quitButton = new QuitButton(ctrl, (SDL_Rect){
			10, 10,
			60, 17
		}, "Quit");
		drawables.push_back(quitButton);
		clickables.push_back(quitButton);

		chirpButton = new ChirpButton(rend, (SDL_Rect){
			10, 33,
			60, 17
		}, "Chirp");
		drawables.push_back(chirpButton);
		clickables.push_back(chirpButton);
	}

	~LivingRoomScene(){
		if(Mix_PlayingMusic())
			Mix_HaltMusic();

		delete quitButton;
		delete chirpButton;
	}
};
