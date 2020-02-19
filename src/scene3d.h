/*
	3D Scene
	mperron (2020)
*/
#define RAD_TO_DEG(r) ((r) / PI * 180)

class Scene3D : public Scene {
private:

public:
	class Radian {
		double value;

	public:
		static double normalize(double value){
			while(value >= (2 * PI))
				value -= (2 * PI);

			while(value < 0)
				value += (2 * PI);

			return value;
		}

		Radian(double value){
			this->value = normalize(value);
		}

		double operator + (const double &d){
			return value + d;
		}

		double operator - (const Radian &other){
			double reverse = normalize(PI + value);
			bool left = false;

			if(reverse > value){
				// Left is on the outside.
				if((other.value <= value) || (other.value > reverse))
					left = true;
			} else {
				// Left is on the inside.
				if((other.value <= value) && (other.value > reverse))
					left = true;
			}

			if(left){
				if(other.value > value)
					return -(value + (2 * PI) - other.value);

				return -(value - other.value);
			}

			if(other.value < value)
				return ((2 * PI) - value + other.value);

			return (other.value - value);
		}

		inline double getValue(){
			return value;
		}
	};

	struct coord {
		double x, y, z;

		double distance_to(const coord &other){
			coord diff = *this - other;

			return sqrt((diff.x * diff.x) + (diff.z * diff.z) + (diff.y * diff.y));
		}

		Radian angle_y(){
			Radian r(atan2(y, sqrt((x * x) + (z * z))));

			return r;
		}

		Radian angle_xz(){
			Radian r(atan2(z, x));

			return r;
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

			// Give whichever direction is smaller a lesser FOV.
			maxangle_w = maxangle_h = maxangle;
			if(w > h){
				maxangle_h = maxangle_h / w * h;
			} else if (h > w){
				maxangle_w = maxangle_w / h * w;
			}
		}

		// Get the x,y coordinates of a pixel on screen to represent this visible vertex.
		pixel vertex_screenspace(coord vertex){
			coord rel = vertex - pos;

			double yaw = (point.angle_xz() - rel.angle_xz());
			double pitch = (point.angle_y() - rel.angle_y());

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
		int drawLine(SDL_Renderer *rend, coord a, coord b){
			static int id = 0;

			{
				Radian point_xz = point.angle_xz();
				double yaw_a = (point_xz - (a - pos).angle_xz());
				double yaw_b = (point_xz - (b - pos).angle_xz());

				if((yaw_a > (PI / 2)) && (yaw_b < -(PI / 2)))
					return (id - 1);
				if((yaw_b > (PI / 2)) && (yaw_a < -(PI / 2)))
					return (id - 1);
			}

			pixel from = vertex_screenspace(a);
			pixel to = vertex_screenspace(b);

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

			point.y = sin(y);
		}

		void walk(double distance){
			double heading = point.angle_xz().getValue();

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
