class SnowScene {
	class SnowFlake {
		float cx, cy;
		float speed;
		float offset;
		float amplitude;

		char lum;

		SnowScene *scene;

		void reset(bool randomY){
			cx = (rand() % scene->w);
			cy = (randomY ? (rand() % scene->h) : 0);
			speed = (rand() % (scene->speed_max - scene->speed_min) + scene->speed_min);
			offset = (rand() % ((int)(2 * 3.14159f * 1000))) / 1000.0f;
			amplitude = (rand() % scene->sway);
			lum = ((rand() % 0xEF) + 0x10);
		}

		float sway(){
			float cx_s = cx + (sin(offset + cy / 100.0f) * amplitude);

			while(cx_s < 0.0f)
				cx_s += scene->w;
			while(cx_s > scene->w)
				cx_s -= scene->w;

			return cx_s;
		}

	public:
		SnowFlake(SnowScene *scene, bool randomY){
			this->scene = scene;
			reset(randomY);
		}

		void update(float time, float angle){
			cy += (speed + cos(angle) * scene->wind_force) * time;
			cx += (((rand() % 1000) / 1000.0f) + sin(angle) * scene->wind_force) * time;

			if(cy > (scene->h + 2)){
				reset(false);
			} else {
				SDL_SetRenderDrawColor(scene->rend, lum, lum, lum, 0xa0);
				SDL_RenderDrawPoint(scene->rend, sway(), cy);
			}
		}
	};

	SDL_Renderer *rend;

	int w, h;
	int speed_min, speed_max;
	int sway;
	int wind_angle, wind_force;
	int count;

	list<SnowFlake*> flakes;

	float ticks;
public:
	SnowScene(
		SDL_Renderer *rend,
		int w, int h,
		int speed_min, int speed_max, int sway,
		int wind_angle, int wind_force,
		int count
	){
		this->rend = rend;
		this->w = w;
		this->h = h;
		this->speed_min = speed_min;
		this->speed_max = speed_max;
		this->sway = sway;
		this->wind_angle = wind_angle;
		this->wind_force = wind_force;
		this->count = count;

		for(int i = 0; i < count; i++)
			flakes.push_front(new SnowFlake(this, true));

		ticks = (SDL_GetTicks() / 1000.0f);
	}

	int update(int ticks){
		float time = (ticks / 1000.0f);

		float angle = (wind_angle / 180.0f) * 3.14159f;
		for(SnowFlake *flake : flakes)
			flake->update(time, angle);

		// Return the number of active particles.
		return count;
	}
};
