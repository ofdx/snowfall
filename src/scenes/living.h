class LivingRoomScene : public Scene {
	class QuitButton : public Button {
	public:
		QuitButton(SDL_Renderer *rend, SDL_Rect click_region) : Button(rend, click_region) {}

		void action(){
			exit(0);
		}
	} *quitButton;

	class ChirpButton : public Button {
		Mix_Chunk *atari = NULL;

	public:
		ChirpButton(SDL_Renderer *rend, SDL_Rect click_region) : Button(rend, click_region) {
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

		void action(){
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

		quitButton = new QuitButton(rend, (SDL_Rect){
			10, 10,
			60, 20
		});
		drawables.push_back(quitButton);
		clickables.push_back(quitButton);

		chirpButton = new ChirpButton(rend, (SDL_Rect){
			10, 40,
			60, 20
		});
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
