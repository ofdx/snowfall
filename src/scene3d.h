/*
	3D Scene
	mperron (2020)
*/
#define RAD_TO_DEG(r)      ((r) / PI * 180)
#define SQUARE(x)          ((x) * (x))
#define MAX_DRAW_DISTANCE  100.0

typedef unsigned char byte_t;

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

		double operator + (const double &d) const {
			return value + d;
		}

		double operator - (const Radian &other) const {
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

		inline double getValue() const {
			return value;
		}
	};

	struct coord {
		double x, y, z;

		double distance_to(const coord &other) const {
			coord diff = *this - other;

			return sqrt(SQUARE(diff.x) + SQUARE(diff.z) + SQUARE(diff.y));
		}

		Radian angle_y() const {
			Radian r(atan2(y, sqrt(SQUARE(x) + SQUARE(z))));

			return r;
		}

		Radian angle_xz() const {
			Radian r(atan2(z, x));

			return r;
		}

		coord operator * (const double &factor) const {
			return (coord){
				this->x * factor,
				this->y * factor,
				this->z * factor
			};
		}

		coord operator / (const int &divisor) const {
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
		coord operator + (const coord &other) const {
			return (coord){
				this->x + other.x,
				this->y + other.y,
				this->z + other.z
			};
		}
		coord operator - (const coord &other) const {
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

		bool operator == (const coord &other) const {
			return (
				(this->x == other.x) &&
				(this->y == other.y) &&
				(this->z == other.z)
			);
		}

		string display() const {
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

		string display() const {
			stringstream ret;

			ret
				<< "("
				<< x << ", "
				<< y
				<< ")";

			return ret.str();
		}

		inline double distance_to(const pixel &other) const {
			return sqrt((double)SQUARE(other.x - x) + (double)SQUARE(other.y - y));
		}

		bool operator >= (const pixel &other) const {
			return !(*this < other);
		}
		bool operator < (const pixel &other) const {
			return ((y == other.y) ? (x < other.y) : (y < other.y));
		}
	};

	class Camera : public Clickable {
		bool mlook_active = false;

	public:
		double maxangle_w, maxangle_h;
		coord pos, point;
		int w, h;
		vector<byte_t> screenspace_px;
		vector<double> screenspace_zb;
		SDL_Texture *screenspace_tx;
		SDL_Renderer *rend;

		Camera(SDL_Renderer *rend, coord pos, coord point, int w, int h, double maxangle) :
			Clickable(),
			screenspace_px(SCREEN_WIDTH * SCREEN_HEIGHT * 4, 0),
			screenspace_zb(SCREEN_WIDTH * SCREEN_HEIGHT, MAX_DRAW_DISTANCE)
		{
			this->rend = rend;
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

			screenspace_tx = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
		}

		~Camera(){
			SDL_DestroyTexture(screenspace_tx);
		}

		// Get the x,y coordinates of a pixel on screen to represent this visible vertex.
		pixel vertex_screenspace(const coord &vertex) const {
			coord rel = vertex - pos;

			double yaw = (rel.angle_xz() - point.angle_xz());
			double pitch = (rel.angle_y() - point.angle_y());

			return (pixel){
				x: w - (int)((w / 2) + (yaw / (2 * maxangle_w) * w)),
				y: h - (int)((h / 2) + (pitch / (2 * maxangle_h) * h))
			};
		}

		bool pixel_visible(const pixel &px) const {
			if(
				(px.x >= 0) && (px.x < w) &&
				(px.y >= 0) && (px.y < h)
			)
				return true;

			return false;
		}

		// Turn the camera the specified number of radians around the Y-axis.
		void yaw(const double &delta){
			double xz = point.angle_xz() + delta;

			point = (coord){ cos(xz), point.y, sin(xz) };
		}

		// Pitch the camera up or down the specified number of radians.
		void pitch(const double &delta){
			double y = point.angle_y() + delta;

			if(y < PI){
				if(y > (PI / 8))
					y = (PI / 8);
			} else {
				if(y < ((2 * PI) - (PI / 8)))
					y = ((2 * PI) - (PI / 8));
			}

			point.y = sin(y) * sqrt(SQUARE(point.x) + SQUARE(point.y) + SQUARE(point.z));
		}

		void walk(const double &distance){
			double heading = point.angle_xz().getValue();

			pos += (coord){ distance * cos(heading), 0, distance * sin(heading) };
		}

		void mlook(const bool &active){
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
					break;
			}
		}

		void draw_frame(){
			// Update the screen texture and draw it.
			SDL_UpdateTexture(screenspace_tx, NULL, &screenspace_px[0], SCREEN_WIDTH * 4);
			SDL_RenderCopy(rend, screenspace_tx, NULL, NULL);

			// Set the entire screen buffer to a background color.
			const byte_t fill[4] = { 0x10, 0x29, 0xad, 0xff }; // BGRA
			for(int i = 0; i < (4 * SCREEN_WIDTH * SCREEN_HEIGHT); i++){
				screenspace_px[i] = fill[i % 4];

				if(!(i % 4))
					screenspace_zb[i / 4] = MAX_DRAW_DISTANCE;
			}

		}
	} *cam;

	struct Renderable : public Drawable {
		Camera *cam;

		Renderable(Camera *cam) : Drawable(cam->rend) {
			this->cam = cam;
		}
	};

	struct Mesh : public Renderable {
		struct Face {
			vector<int> vertIds;
			Mesh *mesh;
			byte_t *fill;

			Face(Mesh *mesh, vector<int> vertIds){
				this->mesh = mesh;
				this->vertIds = vertIds;

				const byte_t fill_default[4] = { 0x99, 0x66, 0x33, 0xff };

				fill = (byte_t*) calloc(4, sizeof(byte_t));
				memcpy(fill, fill_default, 4);
			}

			void set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a){
				const byte_t color[4] = { b, g, r, a };

				memcpy(fill, color, 4);
			}
		};

		vector<coord> vertices;
		list<Face*> faces;

		unordered_map<int, pixel> vertIdToScreen;
		pixel *scanlines;
		coord *scanlines_coords;
		int y_min, y_max;

		void resetScanlines(){
			for(int line = y_min; line <= y_max; line++)
				scanlines[line] = (pixel){ SCREEN_WIDTH, 0 };

			y_min = SCREEN_HEIGHT - 1;
			y_max = 0;
		}

		Mesh(Camera *cam, vector<coord> vertices, list<vector<int>> faces) :
			Renderable(cam)
		{
			this->vertices = vertices;

			for(vector<int> vertIds : faces){
				Face *f = new Face(this, vertIds);

				this->faces.push_back(f);
			}

			scanlines = (pixel*) calloc(SCREEN_HEIGHT, sizeof(pixel));
			scanlines_coords = (coord*) calloc(SCREEN_HEIGHT * 2, sizeof(coord));
			y_min = 0;
			y_max = SCREEN_HEIGHT - 1;
			resetScanlines();
		}
		~Mesh(){
			free(scanlines);
			free(scanlines_coords);

			for(Face *f : faces)
				delete f;
		}

		void translate(const coord &delta){
			for(coord &c : vertices)
				c = c + delta;
		}

		// Set the fill color for all faces of this Mesh.
		void set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a){
			for(Face *face : faces)
				face->set_color_fill(r, g, b, a);
		}

		void populateScreenspace(){
			for(int i = 0, len = vertices.size(); i < len; i++)
				vertIdToScreen[i] = cam->vertex_screenspace(vertices[i]);
		}

		// Draw a line on the screen to connect two pixels.
		void drawLine(const int &vert_a, const int &vert_b){
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

			double x = from.x;
			double y = from.y;
			double dx = to.x - from.x;
			double dy = to.y - from.y;
			double step = ((abs(dx) >= abs(dy)) ? abs(dx) : abs(dy));
			coord coord_step = (b - a) / step;

			dx /= step;
			dy /= step;

			const byte_t fill[4] = { 0x00, 0x00, 0x00, 0xff };
			for(int i = 1; i <= step; i++){
				pixel px = { (int)x, (int)y };

				if(px.y > y_max)
					y_max = px.y;
				if(px.y < y_min)
					y_min = px.y;

				if((px.y >= 0) && (px.y < SCREEN_HEIGHT)){
					pixel bounds = scanlines[px.y];

					if(px.x < bounds.x){
						bounds.x = px.x;
						scanlines_coords[2 * px.y] = a + (coord_step * i);
					}

					if(px.x > bounds.y){
						bounds.y = px.x;
						scanlines_coords[2 * px.y + 1] = a + (coord_step * i);
					}

					scanlines[px.y] = bounds;
				}

				if((px.x >= 0) && (px.y >= 0) && (px.x < SCREEN_WIDTH) && (px.y < SCREEN_HEIGHT)){
					int offset = (SCREEN_WIDTH * px.y + px.x);
					double distance = cam->pos.distance_to(a + (coord_step * i));

					// Draw this pixel if there isn't already one in front of it.
					if(distance < cam->screenspace_zb[offset]){
						memcpy(&cam->screenspace_px[offset * 4], fill, 4);
						cam->screenspace_zb[offset] = distance;
					}
				}

				x += dx;
				y += dy;
			}
		}

		void draw_face(const Face &face){
			resetScanlines();

			// Draw the border, and build a set of pixel coordinates that
			// represent the outline.
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);
			for(int i = 0, len = face.vertIds.size(); i < len; i++)
				drawLine(face.vertIds[i], face.vertIds[((i == len - 1) ? 0 : (i + 1))]);

			if(y_min < 0)
				y_min = 0;
			if(y_max > (SCREEN_HEIGHT - 1))
				y_max = (SCREEN_HEIGHT - 1);

			// Fill each line.
			if(y_min < y_max){
				const byte_t fill_black_data[4] = { 0x00, 0x00, 0x00, face.fill[3] };

				for(int line = y_min; line <= y_max; line++){
					pixel bounds = scanlines[line];
					coord coord_left = scanlines_coords[2 * line];
					coord coord_delta = (scanlines_coords[2 * line + 1] - coord_left) / (bounds.y - bounds.x);
					bool fill_black = false;

					if((line == y_min) || (line == y_max))
						fill_black = true;

					for(int x = bounds.x + 1; x < bounds.y; x++){
						if(((x == (bounds.x + 1)) || (x == (bounds.y - 1))) && !((line == y_min) || (line == y_max)))
							fill_black = true;

						if((x >= 0) && (line >= 0) && (x < SCREEN_WIDTH) && (line < SCREEN_HEIGHT)){
							const unsigned int offset = (SCREEN_WIDTH * line + x);
							double distance = cam->pos.distance_to(coord_left + (coord_delta * (x - bounds.x)));

							// Draw this pixel if there isn't already one in front of it.
							if(distance < cam->screenspace_zb[offset]){
								memcpy(&cam->screenspace_px[offset * 4], (fill_black ? fill_black_data : face.fill), 4);
								cam->screenspace_zb[offset] = distance;
							}
						}

						if((x == (bounds.x + 1)) && !((line == y_min) || (line == y_max)))
							fill_black = false;
					}
				}
			}
		}

		virtual void draw(int ticks){
			populateScreenspace();

			// Sort faces by distance to the camera. Far faces are drawn first.
			for(Face *face : faces)
				draw_face(*face);
		}
	};

	// Objects
	list<Mesh*> drawable_meshes;

	virtual ~Scene3D(){}

	virtual void draw(int ticks){
		// Draw each mesh. The order doesn't matter because the draw function
		// has a z-buffer.
		for(Mesh *mesh : drawable_meshes)
			mesh->draw(ticks);

		// Copy the frame buffer to the screen.
		cam->draw_frame();
		
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
