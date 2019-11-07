class LivingRoomScene : public Scene {
	class QuitButton : public Button {
	public:
		QuitButton(SDL_Renderer *rend, SDL_Rect click_region) : Button(rend, click_region) { }

		void action(){
			exit(0);
		}
	};

public:
	LivingRoomScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "living/1.bmp");

		QuitButton *quitButton = new QuitButton(rend, (SDL_Rect){
			10, 10,
			60, 20
		});

		drawables.push_back(quitButton);
		clickables.push_back(quitButton);
	}
};
