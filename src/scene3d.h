/*
	3D Scene
	mperron (2020)
*/
#define RAD_TO_DEG(r) ((r) / PI * 180)

class Scene3D : public Scene {
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
			double reverse = normalize(PI + other.value);
			bool left = false;

			if(reverse > other.value){
				// Left is on the outside.
				if((value <= other.value) || (value > reverse))
					left = true;
			} else {
				// Left is on the inside.
				if((value <= other.value) && (value > reverse))
					left = true;
			}

			if(left){
				if(value > other.value)
					return -(other.value + (2 * PI) - value);

				return -(other.value - value);
			}

			if(value < other.value)
				return ((2 * PI) - other.value + value);

			return (value - other.value);
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
			return !(*this < other);
		}
		bool operator < (const pixel &other){
			long long lv = x, rv = other.x;

			lv |= (((long long) y) << 32);
			rv |= (((long long) other.y) << 32);

			return (lv < rv);
		}
	};

	class Camera : public Clickable {
		double maxangle_w, maxangle_h;
		bool mlook_active = false;

	public:
		coord pos, point;
		int w, h;

		Camera(coord pos, coord point, int w, int h, double maxangle) :
			Clickable()
		{
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

			double yaw = (rel.angle_xz() - point.angle_xz());
			double pitch = (rel.angle_y() - point.angle_y());

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

		// Turn the camera the specified number of radians around the Y-axis.
		void yaw(double delta){
			double xz = point.angle_xz() + delta;

			point = (coord){ cos(xz), point.y, sin(xz) };
		}

		// Pitch the camera up or down the specified number of radians.
		void pitch(double delta){
			double y = point.angle_y() + delta;

			if(y < PI){
				if(y > (PI / 8))
					y = (PI / 8);
			} else {
				if(y < ((2 * PI) - (PI / 8)))
					y = ((2 * PI) - (PI / 8));
			}

			point.y = sin(y) * sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
		}

		void walk(double distance){
			double heading = point.angle_xz().getValue();

			pos += (coord){ distance * cos(heading), 0, distance * sin(heading) };
		}

		void mlook(bool active){
			if(mlook_active != active)
				SDL_SetRelativeMouseMode(active ? SDL_TRUE : SDL_FALSE);

			mlook_active = active;
		}
		bool mlook_toggle(){
			mlook(!mlook_active);

			return mlook_active;
		}

		virtual void check_mouse(SDL_Event event){
			switch(event.type){
				case SDL_MOUSEMOTION:
					if(mlook_active){
						yaw(-(event.motion.xrel / (16.0 * render_scale)));
						pitch(-(event.motion.yrel / (40.0 * render_scale)));
					}
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					if(event.button.button & SDL_BUTTON(SDL_BUTTON_RIGHT))
						mlook((event.button.state == SDL_PRESSED));
					break;
			}
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

	class Mesh : public Renderable {
		struct Face {
			vector<int> vertIds;

			struct color {
				unsigned char r, g, b, a;
			} color_fill;

			Face(vector<int> vertIds){
				this->vertIds = vertIds;

				// FIXME debug
				color_fill = (color){
					0xff, 0xff, 0xff,
					0xff
				};
			}
		};

		vector<coord> vertices;
		list<Face> faces;

		unordered_map<int, pixel> vertIdToScreen;

	public:
		Mesh(SDL_Renderer *rend, Camera *cam, vector<coord> vertices, list<vector<int>> faces) :
			Renderable(rend, cam)
		{
			this->vertices = vertices;

			for(vector<int> vertIds : faces){
				Face f(vertIds);

				this->faces.push_back(f);
			}
		}

		void translate(coord delta){
			for(coord &c : vertices)
				c = c + delta;
		}

		// Find the average coordinate of all vectors in a face.
		coord face_avg(vector<int> face){
			coord avg = { 0, 0, 0 };
			short verts = face.size();

			if(verts){
				for(int vert : face)
					avg += vertices[vert];

				avg /= verts;
			}

			return avg;
		}

		void populateScreenspace(){
			for(int i = 0, len = vertices.size(); i < len; i++)
				vertIdToScreen[i] = cam->vertex_screenspace(vertices[i]);
		}

		// Draw a line on the screen to connect two pixels.
		void drawLine(int vert_a, int vert_b, set<pixel> &pixel_verts, set<pixel> &pixel_border){
			coord a = vertices[vert_a];
			coord b = vertices[vert_b];

			{
				Radian point_xz = cam->point.angle_xz();
				double yaw_a = ((a - cam->pos).angle_xz() - point_xz);
				double yaw_b = ((b - cam->pos).angle_xz() - point_xz);

				if(((yaw_a > (PI / 2)) && (yaw_b < -(PI / 2))) || ((yaw_b > (PI / 2)) && (yaw_a < -(PI / 2))))
					return;
			}

			pixel from = vertIdToScreen[vert_a];
			pixel to = vertIdToScreen[vert_b];

			pixel_verts.insert(from);
			pixel_verts.insert(to);

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
			pixel px_high = { SCREEN_WIDTH, SCREEN_HEIGHT };

			for(int i = 1; i <= step; i++){
				pixel px = { (int)x, (int)y };
				pixel_border.insert(px);

				if((px >= px_low) && (px < px_high))
					SDL_RenderDrawPoint(rend, px.x, px.y);

				x += dx;
				y += dy;
			}
		}

		virtual void draw(int ticks){
			populateScreenspace();

			// Sorted faces by distance to camera.
			multimap<double, Face, greater<double>> draw_sequence;

			// Sort faces by distance to the camera. Far faces are drawn first.
			for(Face face : faces)
				draw_sequence.insert(pair<double, Face>(cam->pos.distance_to(face_avg(face.vertIds)), face));

			// Draw faces
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);
			for(auto it : draw_sequence){
				Face face = it.second;

				set<pixel> pixel_verts;
				set<pixel> pixel_border;

				// Draw the border, and build a set of pixel coordinates that
				// represent the outline.
				for(int i = 0, len = face.vertIds.size(); i< len; i++){
					drawLine(
						face.vertIds[i],
						face.vertIds[((i == len - 1) ? 0 : (i + 1))],
						pixel_verts,
						pixel_border
					);
				}

				pixel px_low = { SCREEN_WIDTH, SCREEN_HEIGHT };
				pixel px_high = { 0, 0 };

				// Find the X and Y min and max values.
				for(pixel px : pixel_border){
					if(px.x < px_low.x)
						px_low.x = px.x;
					if(px.y < px_low.y)
						px_low.y = px.y;

					if(px.x > px_high.x)
						px_high.x = px.x;
					if(px.y > px_high.y)
						px_high.y = px.y;
				}

				// Scan and fill. Assumes that any given y-scanline has one
				// contiguous fill section [x_start, x_finish].
				if(px_low < px_high){
					SDL_SetRenderDrawColor(rend, 0xff, 0xff, 0xff, 0xff);

					// Make sure we start outside of the shape.
					px_low.x -= 1;

					for(int y = px_low.y; y <= px_high.y; y++){
						int x_start = -1, x_finish = -1;

						for(int x = px_low.x; x <= px_high.x; x++){
							pixel px = { x, y };

							// Find left pixel x_start
							if(pixel_border.count(px)){
								while(++x <= px_high.x){
									px.x = x;

									if(!pixel_border.count(px)){
										x_start = x;
										break;
									}
								}

								// Find right pixel x_finish
								if(x_start > 0){
									while(++x <= px_high.x){
										px.x = x;

										if(pixel_border.count(px)){
											x_finish = (x - 1);
											break;
										}
									}
								}

								break;
							}

						}

						// Found a left and right boundary for the fill, so
						// actually fill this scanline.
						if(x_finish > 0)
							for(int x = x_start; x <= x_finish; x++)
								SDL_RenderDrawPoint(rend, x, y);
					}
				}
			}

			// Draw vertices
			SDL_SetRenderDrawColor(rend, 0, 0xff, 0xff, 0xff);
			for(int i = 0, len = vertices.size(); i < len; i++){
				pixel px = vertIdToScreen[i];

				if(cam->pixel_visible(px))
					SDL_RenderDrawPoint(rend, px.x, px.y);
			}
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

// Required by STL <set>.
inline bool operator < (const Scene3D::pixel &l, const Scene3D::pixel &r){
	long long lv = l.x, rv = r.x;

	lv |= (((long long) l.y) << 32);
	rv |= (((long long) r.y) << 32);

	return (lv < rv);
}
