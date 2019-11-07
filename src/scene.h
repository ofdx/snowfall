/*
	Scene
	mperron (2019)

	A base class from which all of the various screens in the game can
	be derived.
*/
class Scene : public Drawable {
protected:
	SDL_Texture *bg = NULL;
	list<Drawable*> drawables;

	Scene(SDL_Renderer *rend) : Drawable(rend) {}

public:
	virtual ~Scene(){
		if(bg)
			SDL_DestroyTexture(bg);
	}

	virtual void draw(int ticks){
		// Draw the background image.
		if(bg)
			SDL_RenderCopy(rend, bg, NULL, NULL);

		// Draw any drawable elements (buttons, etc.)
		for(auto drawable : drawables)
			drawable->draw(ticks);
	}
};
