/*
	3D Scene
	mperron (2020)
*/
class Scene3D : public Scene {
private:

public:
	struct coord {
		double x, y, z;

		double distance_to(const coord &other){
			coord diff = *this - other;

			return sqrt((diff.x * diff.x) + (diff.z * diff.z) + (diff.y * diff.y));
		}

		double atan2p(double y, double x){
			double a = atan2(y, x);

			return ((a < 0) ? ((2 * PI) + a) : a);
		}

		double angle_y(){
			return atan2p(y, sqrt((x * x) + (z * z)));
		}

		double angle_xz(){
			return atan2p(z, x);
		}

		coord operator / (const int &divisor){
			return (coord){
				this->x / divisor,
				this->y / divisor,
				this->z / divisor
			};
		}
		coord operator /= (const int &divisor){
			*this = *this / divisor;

			return *this;
		}

		coord operator += (const coord &other){
			*this = *this + other;

			return *this;
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
			stringstream ret;

			ret
				<< "("
				<< this->x << ", "
				<< this->y << ", "
				<< this->z
				<< ")";

			return ret.str();
		}
	};

	struct pixel {
		int x, y;

		string display(){
			stringstream ret;

			ret
				<< "("
				<< x << ", "
				<< y
				<< ")";

			return ret.str();
		}

		bool operator >= (const pixel &other){
			return ((x >= other.x) && (y >= other.y));
		}
		bool operator < (const pixel &other){
			return ((x < other.x) && (y < other.y));
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

			double yaw = (rel_xz - point_xz);
			double pitch = (rel_y - point_y);

			return (pixel){
				x: w - (int)((w / 2) + (yaw / (2 * maxangle_w) * w)),
				y: h - (int)((h / 2) + (pitch / (2 * maxangle_h) * h))
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

		// Draw a line on the screen to connect two pixels.
		int drawLine(SDL_Renderer *rend, pixel from, pixel to){
			static int id = 0;

			double dx = to.x - from.x;
			double dy = to.y - from.y;
			double step = ((abs(dx) >= abs(dy)) ? abs(dx) : abs(dy));
			double x, y;

			dx /= step;
			dy /= step;

			x = from.x;
			y = from.y;

			list<pixel> output;

			pixel px_low = { 0, 0 };
			pixel px_high = { w, h };

			for(int i = 1; i <= step; i++){
				pixel px = { (int)x, (int)y };

				if((px >= px_low) && (px < px_high)){
					SDL_RenderDrawPoint(rend, px.x, px.y);
				}

				x += dx;
				y += dy;
			}

			return id++;
		}

		// Turn the camera the specified number of radians around the Y-axis.
		void yaw(double delta){
			double xz = point.angle_xz() + delta;

			point = (coord){ cos(xz), point.y, sin(xz) };
		}

		// Pitch the camera up or down the specified number of radians.
		void pitch(double delta){
			double y = point.angle_y() + delta;

			if(y > (PI / 2))
				y = (PI / 2);
			if(y < -(PI / 2))
				y = -(PI / 2);

			point.y = sin(y);
		}

		void walk(double distance){
			double heading = point.angle_xz();

			pos += (coord){ distance * cos(heading), 0, distance * sin(heading) };
		}
	};

	class Renderable : public Drawable {
	protected:
		Camera *cam;

	public:
		Renderable(SDL_Renderer *rend, Camera *cam) : Drawable(rend) {
			this->cam = cam;
		}
	};

	virtual ~Scene3D(){}

	virtual void draw(int ticks){
		// Clear the screen.
		SDL_SetRenderDrawColor(rend, 0xad, 0x29, 0x10, 255);
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
