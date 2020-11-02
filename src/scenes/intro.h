class IntroSplashScene : public Scene {
	int ticks_total = 0;

	int ticks_tocreditshow   = 1200;
	int ticks_tocreditchange = 1800;

	PicoText *text_credit_me;

	class KrakCircle :
		public Drawable,
		public Movable
	{
		SDL_Texture *krakcircle;

	public:
		KrakCircle(SDL_Renderer *rend) :
			Drawable(rend),
			Movable(0, 0)
		{
			krakcircle = textureFromBmp(rend, "krakcircle.bmp", true);
			movable_time = 1000;
		}

		~KrakCircle(){
			SDL_DestroyTexture(krakcircle);
		}

		void draw(int ticks){
			static bool move_started = false;
			static int ticks_total = 0;

			if(!move_started && ticks_total > 300){
				translate(-90, 0);
				move_started = true;
			}

			movable_update(ticks);
			ticks_total += ticks;

			SDL_Rect draw_rect = {
				movable_x(), movable_y(),
				SCREEN_WIDTH, SCREEN_HEIGHT
			};

			SDL_RenderCopy(rend, krakcircle, NULL, &draw_rect);
		}
	} *kc;

public:
	IntroSplashScene(Scene::Controller *ctrl) : Scene(ctrl) {
		text_credit_me = new PicoText(rend, (SDL_Rect){
			(SCREEN_WIDTH / 2 + 10), (SCREEN_HEIGHT / 2 - 9),
			SCREEN_WIDTH, 25
		}, "");
		text_credit_me->set_color(0x61, 0x34, 0x80);
		drawables.push_back(text_credit_me);

		// Immediately play stinger sound.
		try {
			Mix_PlayChannel(-1, FileLoader::get("sound/stinger.wav")->sound(), 0);
		} catch(...){
			cout << "Failed to find file: stinger.wav" << endl;
		}

		drawables.push_back(kc = new KrakCircle(rend));

		// Disable mouse to hide the cursor.
		ctrl->mouse_enabled = false;
	}

	~IntroSplashScene(){
		delete text_credit_me;
		delete kc;

		// Show mouse cursor again.
		ctrl->mouse_enabled = true;
	}

	void draw(int ticks){
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xff);
		SDL_RenderClear(rend);

		Scene::draw(ticks);
		ticks_total += ticks;

		// A game by:
		if((ticks_tocreditshow -= ticks) <= 0){
			ticks_tocreditshow = 0;
			text_credit_me->set_message("A game by:");
		}

		// Michael Perron
		if((ticks_tocreditchange -= ticks) <= 0){
			ticks_tocreditchange = 0;
			text_credit_me->set_message(
				"A game by:\n"
				"Mike Perron"
			);
		}

		// Wait about three seconds.
		if(ticks_total > 2950)
			ctrl->set_scene(Scene::create(ctrl, "forest"));
	}
};
