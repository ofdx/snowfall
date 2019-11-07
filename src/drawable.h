class Drawable {
protected:
	SDL_Renderer *rend;

	Drawable(SDL_Renderer *rend){
		this->rend = rend;
	}

public:
	virtual void draw(int ticks) = 0;

	virtual ~Drawable(){}
};
