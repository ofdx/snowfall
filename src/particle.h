class ParticleEffect {
protected:
	SDL_Renderer *rend;
	SDL_Rect area;

	ParticleEffect(SDL_Renderer *rend, SDL_Rect area){
		this->rend = rend;
		this->area = area;
	}

public:
	virtual int update(int ticks){
		return 0;
	}
	virtual ~ParticleEffect(){}
};
