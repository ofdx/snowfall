class HelpScene : public Scene {
private:
	SDL_Texture *bg;

public:
	HelpScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "help.bmp");
	}

	~HelpScene(){
		SDL_DestroyTexture(bg);

		// FIXME debug
		cout << FileLoader::get("desc.txt")->text();
	}

	void draw(int ticks){
		SDL_RenderCopy(rend, bg, NULL, NULL);
	}
};
