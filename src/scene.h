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

	// Slide along x^2 from x=-16...0, mapped onto the difference to-from.
	// Time is milliseconds to complete the transition. Ticks are milliseconds
	// since the last drawn frame. The progress var should be set to 0.0f
	// initially and will be 16.0f when the transition is complete.
	int slide_quad_left(int from, int to, int time, int ticks, float &progress){
		progress += ticks * 16.0f / time;

		if(progress >= 16.0f){
			progress = 16.0f;
			return to;
		}

		float slide = progress - 16.0f;
		return (((to - from) * (256 - (slide * slide)) / 256) + from);
	}

	// Same as slide_quad_left, except that it runs from x=0...16. This means
	// progress is slow at first and then fast.
	int slide_quad_right(int from, int to, int time, int ticks, float &progress){
		progress += ticks * 16.0f / time;

		if(progress >= 16.0f){
			progress = 16.0f;
			return to;
		}

		return (((to - from) * (progress * progress) / 256) + from);
	}

	// Slides from x=0...16, then x=-16...0, creating a curve which is slow at
	// the start and end, but fast in the middle.
	int slide_quad(int from, int to, int time, int ticks, float &progress){
		int width = to - from;

		if(progress < 16.0f){
			// Run the positive side of the curve (x=0...16)
			return (from + slide_quad_right(0, width / 2, time / 2, ticks, progress));
		} else if(progress < 32.0f){
			// Run x=-16...0
			float slide = (progress - 16.0f);
			float ret = ((from + width / 2) + slide_quad_left(0, (width - (width / 2)), time / 2, ticks, slide));

			progress = (slide + 16.0f);
			return ret;
		}

		// Done
		progress = 32.0f;
		return to;
	}

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

		void quit(){
			// Show a confirmation dialog?
			// TODO

			exit(0);
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
