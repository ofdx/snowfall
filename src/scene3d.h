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
		bool mlook_active = false;

	public:
		double maxangle_w, maxangle_h;
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

	struct Renderable : public Drawable {
		Camera *cam;

		Renderable(SDL_Renderer *rend, Camera *cam) : Drawable(rend) {
			this->cam = cam;
		}
	};

	struct Mesh : public Renderable {
		struct Face {
			vector<int> vertIds;
			Mesh *mesh;

			struct color {
				unsigned char r, g, b, a;
			} color_fill;

			Face(Mesh *mesh, vector<int> vertIds){
				this->mesh = mesh;
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
		pixel *scanlines;
		int y_min, y_max;

		void resetScanlines(){
			for(int line = y_min; line <= y_max; line++)
				scanlines[line] = (pixel){ SCREEN_WIDTH, 0 };

			y_min = SCREEN_HEIGHT - 1;
			y_max = 0;
		}

		Mesh(SDL_Renderer *rend, Camera *cam, vector<coord> vertices, list<vector<int>> faces) :
			Renderable(rend, cam)
		{
			this->vertices = vertices;

			for(vector<int> vertIds : faces){
				Face f(this, vertIds);

				this->faces.push_back(f);
			}

			scanlines = (pixel*) calloc(SCREEN_HEIGHT, sizeof(pixel));
			y_min = 0;
			y_max = SCREEN_HEIGHT - 1;
			resetScanlines();
		}
		~Mesh(){
			free(scanlines);
		}

		void translate(coord delta){
			for(coord &c : vertices)
				c = c + delta;
		}

		// Find the nearest face to the camera.
		coord face_min(vector<int> face, Camera *cam){
			double min_dist;
			coord min;

			short verts = face.size();

			if(verts){
				bool first = true;

				for(int vert : face){
					coord v = vertices[vert];

					if(first){
						first = false;
						min = v;
						min_dist = cam->pos.distance_to(min);
					} else {
						double dist = cam->pos.distance_to(v);

						if(dist < min_dist){
							min_dist = dist;
							min = v;
						}
					}
				}
			}

			return min;
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
		void drawLine(int vert_a, int vert_b){
			coord a = vertices[vert_a];
			coord b = vertices[vert_b];

			{
				Radian point_xz = cam->point.angle_xz();
				double yaw_a = ((a - cam->pos).angle_xz() - point_xz);
				double yaw_b = ((b - cam->pos).angle_xz() - point_xz);

				if(
					(abs(yaw_a) > (PI / 2) && abs(yaw_b) > cam->maxangle_w) ||
					(abs(yaw_b) > (PI / 2) && abs(yaw_a) > cam->maxangle_w)
				)
					return;
			}

			pixel from = vertIdToScreen[vert_a];
			pixel to = vertIdToScreen[vert_b];

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

				if(px.y > y_max)
					y_max = px.y;
				if(px.y < y_min)
					y_min = px.y;

				if((px.y >= 0) && (px.y < SCREEN_HEIGHT)){
					pixel bounds = scanlines[px.y];

					if(px.x < bounds.x)
						bounds.x = px.x;
					if(px.x > bounds.y)
						bounds.y = px.x;

					scanlines[px.y] = bounds;
				}

				if((px >= px_low) && (px < px_high))
					SDL_RenderDrawPoint(rend, px.x, px.y);

				x += dx;
				y += dy;
			}
		}

		void draw_face(Face face){
			resetScanlines();

			// Draw the border, and build a set of pixel coordinates that
			// represent the outline.
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);
			for(int i = 0, len = face.vertIds.size(); i< len; i++){
				drawLine(
					face.vertIds[i],
					face.vertIds[((i == len - 1) ? 0 : (i + 1))]
				);
			}

			if(y_min < 0)
				y_min = 0;
			if(y_max > (SCREEN_HEIGHT - 1))
				y_max = (SCREEN_HEIGHT - 1);

			// Fill each line.
			if(y_min < y_max){
				for(int line = y_min; line <= y_max; line++){
					pixel bounds = scanlines[line];

					if((line == y_min) || (line == y_max))
						SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);

					for(int x = bounds.x + 1; x < bounds.y; x++){
						if(((x == (bounds.x + 1)) || (x == (bounds.y - 1))) && !((line == y_min) || (line == y_max)))
							SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);

						if((x >= 0) && (x < SCREEN_WIDTH))
							SDL_RenderDrawPoint(rend, x, line);

						if((x == (bounds.x + 1)) && !((line == y_min) || (line == y_max)))
							SDL_SetRenderDrawColor(rend, 0x33, 0x66, 0x99, 0xff);
					}
				}
			}
		}

		static void draw_faces(multimap<double, Face, greater<double>> &draw_sequence){
			// Draw faces
			for(auto it : draw_sequence){
				Face face = it.second;
				Mesh *mesh = face.mesh;

				mesh->draw_face(face);
			}
		}

		virtual void draw(int ticks){
			populateScreenspace();

			// Sorted faces by distance to camera.
			multimap<double, Face, greater<double>> draw_sequence;

			// Sort faces by distance to the camera. Far faces are drawn first.
			for(Face face : faces)
				draw_sequence.insert(
					pair<double, Face>(
						cam->pos.distance_to(face_avg(face.vertIds)),
						face
					)
				);

			draw_faces(draw_sequence);
		}
	};

	// Objects
	list<Mesh*> drawable_meshes;

	virtual ~Scene3D(){}

	virtual void draw(int ticks){
		// Clear the screen.
		SDL_SetRenderDrawColor(rend, 0xad, 0x29, 0x10, 255);
		SDL_RenderClear(rend);

		// Draw 3D scene.
		{
			multimap<double, Mesh::Face, greater<double>> draw_sequence;

			for(Mesh *mesh : drawable_meshes){
				mesh->populateScreenspace();

				for(Mesh::Face face : mesh->faces)
					draw_sequence.insert(
						pair<double, Mesh::Face>(
							mesh->cam->pos.distance_to(mesh->face_avg(face.vertIds)),
							face
						)
					);
			}

			Mesh::draw_faces(draw_sequence);
		}
		
		// Draw everything else on top.
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
