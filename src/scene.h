/*
	Scene
	mperron (2019)

	A base class from which all of the various screens in the game can
	be derived.
*/
class Scene {
protected:
	SDL_Renderer *rend;

	Scene(SDL_Renderer *rend){
		this->rend = rend;
	}

public:
	virtual void draw(int ticks){}

	virtual ~Scene(){}
};
