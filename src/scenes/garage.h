class GarageScene : public Scene {
private:
	SDL_Texture *bg;

public:
	GarageScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "garage.bmp");
	}

	~GarageScene(){
		SDL_DestroyTexture(bg);
	}

	void draw(int ticks){
		// Draw the background image.
		SDL_RenderCopy(rend, bg, NULL, NULL);
	}
};
