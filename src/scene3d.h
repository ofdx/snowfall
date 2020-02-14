/*
	3D Scene
	mperron (2020)
*/
class Scene3D : public Scene {
private:

public:
	struct coord {
		double x, y, z;

		// Canonicalize angle [0,2*PI).
		static double angle_canon(double theta){
			while(theta < 0)
				theta += (2 * PI);
			while(theta >= (2 * PI))
				theta -= (2 * PI);

			return theta;
		}

		double angle_y(){
			return coord::angle_canon(atan2(y, sqrt((x * x) + (z * z))));
		}

		double angle_xz(){
			return coord::angle_canon(atan2(z, x));
		}

		coord operator + (const coord &other){
			return (coord){
				this->x + other.x,
				this->y + other.y,
				this->z + other.z
			};
		}
		coord operator - (const coord &other){
			return (coord){
				this->x - other.x,
				this->y - other.y,
				this->z - other.z
			};
		}
		coord operator = (const coord &other){
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;

			return *this;
		}

		bool operator == (const coord &other){
			return (
				(this->x == other.x) &&
				(this->y == other.y) &&
				(this->z == other.z)
			);
		}

		string display(){
			string ret("(");

			ret += (
				to_string(this->x) + ", " +
				to_string(this->y) + ", " +
				to_string(this->z) + ")"
			);

			return ret;
		}
	};

	struct pixel {
		int x, y;

		string display(){
			string ret("(");

			ret += to_string(x) + ", " + to_string(y) + ")";
			return ret;
		}
	};

	class Camera {
		double maxangle_w, maxangle_h;

	public:
		coord pos, point;
		int w, h;

		Camera(coord pos, coord point, int w, int h, double maxangle){
			this->pos = pos;
			this->point = point;
			this->w = w;
			this->h = h;

			// Give whichever direction is larger a greater FOV.
			maxangle_w = maxangle_h = maxangle;
			if(w > h){
				maxangle_w = maxangle_w / h * w;
			} else if (h > w){
				maxangle_h = maxangle_h / w * h;
			}
		}

		// Get the x,y coordinates of a pixel on screen to represent this visible vertex.
		pixel vertex_screenspace(coord vertex){
			coord rel = vertex - pos;

			double
				point_xz = point.angle_xz(),
				point_y = point.angle_y(),
				rel_xz = rel.angle_xz(),
				rel_y = rel.angle_y();

			double yaw = coord::angle_canon(rel_xz - point_xz + maxangle_w);
			double pitch = coord::angle_canon(rel_y - point_y + maxangle_h);

			return (pixel){
				x: w - (int)(yaw / (2 * maxangle_w) * w),
				y: h - (int)(pitch / (2 * maxangle_h) * h)
			};
		}

		bool pixel_visible(pixel px){
			if(
				(px.x >= 0) && (px.x < w) &&
				(px.y >= 0) && (px.y < h)
			)
				return true;

			return false;
		}
	};

	class Renderable : public Drawable {
	protected:
		Camera *cam;

	public:
		Renderable(SDL_Renderer *rend, Camera *cam) : Drawable(rend) {
			this->cam = cam;
		}

		virtual void draw(int ticks){
			// Draw in 3D space.
		}
	};

	virtual ~Scene3D(){}

	virtual void draw(int ticks){
		// Clear the screen.
		SDL_SetRenderDrawColor(rend, 255, 0, 255, 255);
		SDL_RenderClear(rend);
		
		// Draw everything.
		Scene::draw(ticks);
	}

	virtual void check_mouse(SDL_Event event){
		// Check for mouse activity in 3D space.
		// TODO

		// Send mouse events to 2D space.
		Scene::check_mouse(event);
	}

protected:
	Scene3D(Controller *ctrl) : Scene(ctrl) {}

};
