class ForestScene : public Scene {
	SDL_Texture *forest;
	SDL_Rect pan;

	float pan_vertical = 0.0f;
	bool panning = true;
	float fade_buttons = 0.0f;
	float fade_title = 0.0f;

	float slide_card = 0.0f;
	bool slide_card_dir = false;

	ParticleEffect *snow;

	class PlayButton : public Button {
		Scene::Controller *ctrl;

	public:
		PlayButton(Scene::Controller *ctrl, int x, int y, string text) : Button(ctrl->renderer(), x, y, 1, text.size(), text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->set_scene(Scene::create(ctrl, "living"));
		}
	} *playButton;

	class QuitButton : public Button {
		Scene::Controller *ctrl;

	public:
		QuitButton(Scene::Controller *ctrl, int x, int y, string text) : Button(ctrl->renderer(), x, y, 1, text.size(), text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->quit();
		}
	} *quitButton;

	PicoText *title;
	Mix_Music *music;

	CardPanel<PlayingCard> *card_panel;

public:
	ForestScene(Scene::Controller *ctrl) : Scene(ctrl) {
		forest = textureFromBmp(rend, "pict0007.bmp");
		pan = (SDL_Rect){
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT
		};

		snow = new SnowEffect(
			rend,
			(SDL_Rect){
				0, 0,
				SCREEN_WIDTH, SCREEN_HEIGHT
			},
			10, 50, 40,
			-90, 10,
			150
		);
		drawables.push_back(snow);

		drawables.push_back(quitButton = new QuitButton(ctrl, SCREEN_WIDTH - 32, (SCREEN_HEIGHT - 18), "Quit"));
		quitButton->set_alpha(0x00);

		drawables.push_back(playButton = new PlayButton(ctrl, SCREEN_WIDTH - 88, (SCREEN_HEIGHT - 18), "Begin..."));
		playButton->set_alpha(0x00);

		title = new PicoText(rend, (SDL_Rect){
			5, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT
		}, "Pico Gamo");
		title->set_font("fonts/24x28.bmp", 24, 28);
		title->set_color(0x90, 0x00, 0x00);
		title->set_alpha(0x00);
		drawables.push_back(title);

		// Load and start background music.
		{
			FileLoader *fl = FileLoader::get("t2.wav");

			if(fl){
				music = fl->music();
				Mix_PlayMusic(music, -1);
			} else {
				cout << "Failed to find file: t2.wav" << endl;
			}
		}

		card_panel = new CardPanel<PlayingCard>();
		for(int i = 1; i <= 13; i++)
			card_panel->add(new PlayingCard(rend, 20, 20, PlayingCard::Suit::B, i, true));

		drawables.push_back(card_panel);
		clickables.push_back(card_panel);
	}

	~ForestScene(){
		if(Mix_PlayingMusic())
			Mix_HaltMusic();

		SDL_DestroyTexture(forest);

		delete playButton;
		delete quitButton;

		delete title;

		delete card_panel;

		delete snow;
	}

	void draw(int ticks){
		if(panning){
			// Pan the image from top to bottom.
			pan.y = slide_quad(0, (384 - SCREEN_HEIGHT), 3000, ticks, pan_vertical);

			// Stop panning when we've reached the target.
			if(pan.y == (384 - SCREEN_HEIGHT)){
				panning = false;

				// Make the buttons interactive when the pan completes.
				clickables.push_back(playButton);
				clickables.push_back(quitButton);
			}
		}

		if((fade_title < 32.0f) && (pan_vertical > 8.0f))
			title->set_alpha(slide_quad_right(0x00, 0xcc, 5000, ticks, fade_title));

		if(fade_buttons < 16.0f){
			// Fade in the buttons/text.
			char alpha = slide_quad_right(0x00, 0xff, 3500, ticks, fade_buttons);

			playButton->set_alpha(alpha);
			quitButton->set_alpha(alpha);
		}


		// Cycle through playing cards.
		if(slide_card == 0.0f)
			card_panel->next();

		// Make the card float around spookily.
		PlayingCard *card_king = (PlayingCard*) card_panel->get_active();
		if(slide_card_dir){
			card_king->set_pos(slide_quad((SCREEN_WIDTH - 50), 20, 5000, ticks, slide_card), 8 * sin((slide_card / 8.0f) * 3.14159) + 50);
		} else {
			card_king->set_pos(slide_quad(20, (SCREEN_WIDTH - 50), 5000, ticks, slide_card), 8 * sin((slide_card / 8.0f) * 3.14159) + 50);
		}
		if(slide_card >= 32.0f){
			slide_card_dir = !slide_card_dir;
			slide_card = 0.0f;
		}

		SDL_RenderCopy(rend, forest, &pan, NULL);

		Scene::draw(ticks);
	}
};
