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
	list<Clickable*> clickables;


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

	virtual void check_mouse(SDL_Event event){
		for(auto clickable : clickables)
			clickable->check_mouse(event);
	}

	class Controller : public Drawable {
	public:
		Scene *scene = NULL;

		Controller(SDL_Renderer *rend) : Drawable(rend) {}

		SDL_Renderer *renderer(){
			return rend;
		}

		void set_scene(Scene *scene){
			if(this->scene)
				delete this->scene;

			this->scene = scene;
		}

		void draw(int ticks){
			if(scene)
				scene->draw(ticks);
		}

		void check_mouse(SDL_Event event){
			if(scene)
				scene->check_mouse(event);
		}
	};

protected:
	Controller *ctrl;
	Scene(Controller *ctrl) : Drawable(ctrl->renderer()) {}
};
