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
