class GarageScene : public Scene {
	class LivingRoomButton : public Button {
		Scene::Controller *ctrl;

	public:
		LivingRoomButton(Scene::Controller *ctrl, SDL_Renderer *rend, int x, int y, string text) : Button(rend, x, y, 1, text.length(), text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->set_scene(Scene::create(ctrl, "living"));
		}

	} *livingRoomButton;

	PicoText *label, *email;

public:
	GarageScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "garage.bmp");

		livingRoomButton = new LivingRoomButton(ctrl, rend, SCREEN_WIDTH - 90, 10, "Living Room");
		drawables.push_back(livingRoomButton);
		clickables.push_back(livingRoomButton);

		label = new PicoText(rend, (SDL_Rect){
			10, 40,
			80, 20
		}, "What? I don't know how to tell you...\nYap!! 10-50; 100%\n*<>[]()\n$7752.10");
		label->set_color(0, 0xff, 0);
		label->set_ticks_perchar(50);
		drawables.push_back(label);

		email = new PicoText(rend, (SDL_Rect){
			SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20,
			120, 20
		}, "[mike@krakissi.net]");
		email->set_color(0xff, 0xff, 0);
		drawables.push_back(email);
	}

	~GarageScene(){
		delete livingRoomButton;
		delete label;
		delete email;
	}
};
