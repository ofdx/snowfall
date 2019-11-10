class GarageScene : public Scene {
	class LivingRoomButton : public Button {
		Scene::Controller *ctrl;

	public:
		LivingRoomButton(Scene::Controller *ctrl, SDL_Renderer *rend, SDL_Rect click_region) : Button(rend, click_region) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->set_scene(Scene::create(ctrl, "living"));
		}

	} *livingRoomButton;

public:
	GarageScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "garage.bmp");

		livingRoomButton = new LivingRoomButton(ctrl, rend, (SDL_Rect){
			10, 10,
			60, 20
		});
		drawables.push_back(livingRoomButton);
		clickables.push_back(livingRoomButton);
	}
};
