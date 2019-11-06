class LivingRoomScene : public Scene {
private:
	SDL_Texture *bg;

public:
	LivingRoomScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "living/1.bmp");
	}

	~LivingRoomScene(){
		SDL_DestroyTexture(bg);
	}

	void draw(int ticks){
		// Draw the background image.
		SDL_RenderCopy(rend, bg, NULL, NULL);
	}
};
