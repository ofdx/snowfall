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
		Scene *scene_next = NULL;

	public:
		Scene *scene = NULL;

		Controller(SDL_Renderer *rend) : Drawable(rend) {}

		SDL_Renderer *renderer(){
			return rend;
		}

		void set_scene(Scene *scene){
			this->scene_next = scene;
		}

		void draw(int ticks){
			if(scene_next){
				if(scene)
					delete scene;

				scene = scene_next;
				scene_next = NULL;
			}

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

	class SceneFn {
	public:
		Scene* (*fn)(Scene::Controller*);

		SceneFn(Scene* (*fn)(Scene::Controller*)){
			this->fn = fn;
		}
	};

	static map<string, SceneFn*> scenes;

public:
	static void reg(string, Scene* (*fn)(Controller*));
	static Scene *create(Controller *ctrl, string);
};

map<string, Scene::SceneFn*> Scene::scenes;
void Scene::reg(string name, Scene* (*fn)(Scene::Controller*)){
	scenes[name] = new Scene::SceneFn(fn);
}
Scene *Scene::create(Scene::Controller *ctrl, string name){
	Scene::SceneFn *fn = scenes[name];

	if(fn)
		return fn->fn(ctrl);

	return NULL;
}

template<class T> Scene *scene_create(Scene::Controller *ctrl){
	return new T(ctrl);
}
