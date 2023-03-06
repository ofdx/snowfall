/*
	3D Scene
	mperron (2020)
*/
#ifdef __MINGW64__
	#include "mingw-std-threads/mingw.thread.h"
#else
	#include <thread>
#endif

#include <atomic>

#define RAD_TO_DEG(r)      ((r) / M_PI * 180)
#define SQUARE(x)          ((x) * (x))
#define MAX_DRAW_DISTANCE  100.0
#define MAX_CAM_PITCH      0.35

int const NUM_RENDER_THREADS = 3;

typedef unsigned char byte_t;

// MultiThreadCamera worker function.
void mtCamWorker(int n);

class Scene3D : public Scene {
public:

	class Radian {
		double value;

	public:
		static double Normalize(double value);
		Radian(double value);
		double operator + (const double &d) const;
		double operator - (const Radian &other) const;
		inline double getValue() const;
	};

	struct coord {
		double x, y, z;

		double distance_to(const coord &other) const;
		Radian angle_y() const;
		Radian angle_xz() const;
		coord operator * (const double &factor) const;
		coord operator / (const int &divisor) const;
		coord operator /= (const int &divisor);
		coord operator += (const coord &other);
		coord operator + (const coord &other) const;
		coord operator - (const coord &other) const;
		coord operator = (const coord &other);
		bool operator == (const coord &other) const;
		string display() const;
	};

	struct pixel {
		int x, y;
	};

	class Camera : public Clickable {
		bool mlook_active = false;

	public:
		coord pos, point;
		Radian point_xz = 0, point_y = 0;

		double maxangle_w, maxangle_h;
		int w, h;
		vector<byte_t> screenspace_px;
		vector<double> screenspace_zb;
		SDL_Texture *screenspace_tx;
		SDL_Renderer *rend;

		double max_pitch;

		bool m_oddscanline, m_interlace;

		Camera(SDL_Renderer *rend, coord pos, coord point, int w, int h, double maxangle);
		Camera operator = (Camera const& other);
		virtual ~Camera();

		void set_fov(double maxangle);
		double get_fov();

		pixel vertex_screenspace(const coord &vertex) const;
		void yaw(const double &delta);
		void pitch(const double &delta);
		void walk(const double &distance);
		void mlook(const bool &active);
		bool mlook_toggle();
		virtual void check_mouse(SDL_Event event);
		void clear();
		virtual void draw_frame();
		virtual void cache();
	};

	struct Renderable : public Drawable {
		Camera *cam;

		Renderable(Camera *cam);
		virtual void draw_if_cam(int ticks, Camera const *refCam);
	};

	class MultiThreadCamera : public Camera {
		// Camera for each thread.
		Camera *m_pCams[NUM_RENDER_THREADS] = {};

		// Should be a pointer to the scene's drawable_meshes
		list<Renderable*> *m_pMeshes;

	public:
		struct Shmem {
			std::atomic<bool> m_alive, m_run, m_done;
			Camera *m_cam;
			list<Renderable*> *m_pMeshes;

			Shmem(list<Renderable*> *pMeshes, Camera *cam);
		};

		MultiThreadCamera(SDL_Renderer *rend, coord pos, coord point, int w, int h, double maxangle, list<Renderable*> *pMeshes);
		virtual ~MultiThreadCamera();

		Camera *nextThreadCam();
		void draw_frame() override;
		void cache() override;
	} *cam;

	struct Mesh : public Renderable {
		struct Face {
			vector<int> vertIds;
			Mesh *mesh;
			byte_t *fill;

			Face(vector<int> vertIds, const byte_t fill[4]);
			void set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a);
		};

		vector<coord> vertices;
		list<Face*> faces;

		unordered_map<int, pixel> vertIdToScreen;
		pixel *scanlines;
		coord *scanlines_coords;
		int y_min, y_max;

		void resetScanlines();

		Mesh(Camera *cam, vector<coord> vertices, list<Face*> faces);
		~Mesh();

		static Mesh* load(Camera *cam, string fname);
		void translate(const coord &delta);
		void set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a);
		void populateScreenspace();
		void drawLine(const int &vert_a, const int &vert_b);
		void drawFace(const Face &face);
		virtual void draw(int ticks) override;
	};

	// Objects
	list<Renderable*> drawable_meshes;

	virtual ~Scene3D();

	virtual void draw(int ticks) override;
	virtual void check_mouse(SDL_Event event) override;

protected:
	Scene3D(Controller *ctrl);
};

double Scene3D::Radian::Normalize(double value){
	while(value >= (2 * M_PI))
		value -= (2 * M_PI);

	while(value < 0)
		value += (2 * M_PI);

	return value;
}
Scene3D::Radian::Radian(double value){
	this->value = Normalize(value);
}
double Scene3D::Radian::operator + (const double &d) const {
	return value + d;
}
double Scene3D::Radian::operator - (const Radian &other) const {
	double reverse = Normalize(M_PI + other.value);
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
			return -(other.value + (2 * M_PI) - value);

		return -(other.value - value);
	}

	if(value < other.value)
		return ((2 * M_PI) - other.value + value);

	return (value - other.value);
}
inline double Scene3D::Radian::getValue() const {
	return value;
}


double Scene3D::coord::distance_to(const coord &other) const {
	coord diff = *this - other;

	return sqrt(SQUARE(diff.x) + SQUARE(diff.z) + SQUARE(diff.y));
}
Scene3D::Radian Scene3D::coord::angle_y() const {
	Radian r(atan2(y, sqrt(SQUARE(x) + SQUARE(z))));

	return r;
}
Scene3D::Radian Scene3D::coord::angle_xz() const {
	Radian r(atan2(z, x));

	return r;
}
Scene3D::coord Scene3D::coord::operator * (const double &factor) const {
	return (coord){
		this->x * factor,
		this->y * factor,
		this->z * factor
	};
}
Scene3D::coord Scene3D::coord::operator / (const int &divisor) const {
	return (coord){
		this->x / divisor,
		this->y / divisor,
		this->z / divisor
	};
}
Scene3D::coord Scene3D::coord::operator /= (const int &divisor){
	*this = *this / divisor;

	return *this;
}
Scene3D::coord Scene3D::coord::operator += (const Scene3D::coord &other){
	*this = *this + other;

	return *this;
}
Scene3D::coord Scene3D::coord::operator + (const Scene3D::coord &other) const {
	return (coord){
		this->x + other.x,
		this->y + other.y,
		this->z + other.z
	};
}
Scene3D::coord Scene3D::coord::operator - (const Scene3D::coord &other) const {
	return (coord){
		this->x - other.x,
		this->y - other.y,
		this->z - other.z
	};
}
Scene3D::coord Scene3D::coord::operator = (const Scene3D::coord &other){
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;

	return *this;
}
bool Scene3D::coord::operator == (const Scene3D::coord &other) const {
	return (
		(this->x == other.x) &&
		(this->y == other.y) &&
		(this->z == other.z)
	);
}
string Scene3D::coord::display() const {
	stringstream ret;

	ret
		<< "("
		<< this->x << ", "
		<< this->y << ", "
		<< this->z
		<< ")";

	return ret.str();
}


Scene3D::Camera::Camera(SDL_Renderer *rend, coord pos, coord point, int w, int h, double maxangle) :
	Clickable(),
	pos(pos),
	point(point),
	w(w),
	h(h),
	screenspace_px(SCREEN_WIDTH * SCREEN_HEIGHT * 4, 0),
	screenspace_zb(SCREEN_WIDTH * SCREEN_HEIGHT, MAX_DRAW_DISTANCE),
	rend(rend),
	max_pitch(MAX_CAM_PITCH),
	m_oddscanline(false), m_interlace(false)
{
	set_fov(maxangle);
	screenspace_tx = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	cache();
}
Scene3D::Camera Scene3D::Camera::operator = (Scene3D::Camera const& other){
	mlook_active = other.mlook_active;
	pos = other.pos;
	point = other.point;
	point_xz = other.point_xz;
	point_y = other.point_y;
	maxangle_w = other.maxangle_w;
	maxangle_h = other.maxangle_h;
	w = other.w;
	h = other.h;
	max_pitch = other.max_pitch;
	m_oddscanline = other.m_oddscanline;
	m_interlace = other.m_interlace;

	return *this;
}
Scene3D::Camera::~Camera(){
	SDL_DestroyTexture(screenspace_tx);
}
void Scene3D::Camera::set_fov(double maxangle){
	maxangle_w = maxangle_h = maxangle;

	// Give whichever direction is smaller a lesser FOV.
	if(w > h){
		maxangle_h = maxangle_h / w * h;
	} else if (h > w){
		maxangle_w = maxangle_w / h * w;
	}
}
double Scene3D::Camera::get_fov(){
	return ((maxangle_w > maxangle_h) ? maxangle_w : maxangle_h);
}
Scene3D::pixel Scene3D::Camera::vertex_screenspace(const Scene3D::coord &vertex) const {
	// Get the x,y coordinates of a pixel on screen to represent this visible vertex.
	coord rel = vertex - pos;

	double yaw = (rel.angle_xz() - point_xz);
	double pitch = (rel.angle_y() - point_y);

	return (pixel){
		x: w - (int)((w / 2) + (yaw / (2 * maxangle_w) * w)),
		y: h - (int)((h / 2) + (pitch / (2 * maxangle_h) * h))
	};
}
void Scene3D::Camera::yaw(const double &delta){
	// Turn the camera the specified number of radians around the Y-axis.
	double xz = point_xz + delta;

	point = (coord){ cos(xz), point.y, sin(xz) };
	cache();
}
void Scene3D::Camera::pitch(const double &delta){
	// Pitch the camera up or down the specified number of radians.
	double y = point_y + delta;

	if(y < M_PI){
		if(y > max_pitch)
			y = max_pitch;
	} else {
		if(y < ((2 * M_PI) - max_pitch))
			y = ((2 * M_PI) - max_pitch);
	}

	point.y = sin(y) * sqrt(SQUARE(point.x) + SQUARE(point.y) + SQUARE(point.z));
	cache();
}
void Scene3D::Camera::walk(const double &distance){
	double heading = point_xz.getValue();

	pos += (coord){ distance * cos(heading), 0, distance * sin(heading) };
	cache();
}
void Scene3D::Camera::mlook(const bool &active){
	if(mlook_active != active)
		SDL_SetRelativeMouseMode(active ? SDL_TRUE : SDL_FALSE);

	mlook_active = active;
}
bool Scene3D::Camera::mlook_toggle(){
	mlook(!mlook_active);

	return mlook_active;
}
void Scene3D::Camera::check_mouse(SDL_Event event){
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
void Scene3D::Camera::clear(){
	// Set the entire screen buffer to a background color.
	const byte_t fill[4] = { 0x10, 0x29, 0xad, 0xff }; // BGRA
	for(int i = 0; i < (4 * SCREEN_WIDTH * SCREEN_HEIGHT); i++){
		if(m_interlace){
			int linemod = i % (4 * SCREEN_WIDTH * 2);

			if(!m_oddscanline){
				if(linemod == 0){
					i += (4 * SCREEN_WIDTH - 1);
					continue;
				}
			} else if(linemod == (4 * SCREEN_WIDTH)){
				i += (4 * SCREEN_WIDTH - 1);
				continue;
			}
		}

		screenspace_px[i] = fill[i % 4];

		if(!(i % 4))
			screenspace_zb[i / 4] = MAX_DRAW_DISTANCE;
	}

	m_oddscanline = !m_oddscanline;
}
void Scene3D::Camera::draw_frame(){
	// Update the screen texture and draw it.
	SDL_UpdateTexture(screenspace_tx, NULL, &screenspace_px[0], SCREEN_WIDTH * 4);
	SDL_RenderCopy(rend, screenspace_tx, NULL, NULL);

	// Clear everything to prepare for the next frame.
	clear();
}
void Scene3D::Camera::cache(){
	point_xz = point.angle_xz();
	point_y = point.angle_y();
}


// Shared memory for camera threads.
Scene3D::MultiThreadCamera::Shmem *MtCamMem[NUM_RENDER_THREADS] = {};
Scene3D::MultiThreadCamera::Shmem::Shmem(list<Scene3D::Renderable*> *pMeshes, Scene3D::Camera *cam) :
	m_alive(true),
	m_run(false),
	m_done(false),
	m_cam(cam),
	m_pMeshes(pMeshes)
{ }

Scene3D::MultiThreadCamera::MultiThreadCamera(SDL_Renderer *rend, Scene3D::coord pos, Scene3D::coord point, int w, int h, double maxangle, list<Scene3D::Renderable*> *pMeshes) :
	Scene3D::Camera(rend, pos, point, w, h, maxangle),
	m_pMeshes(pMeshes)
{
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		// Create camera and its shared memory.
		MtCamMem[i] = new Shmem(m_pMeshes, (m_pCams[i] = new Camera(rend, pos, point, w, h, maxangle)));

		// Create camera worker thread.
		std::thread worker(mtCamWorker, i);
		worker.detach();
	}
}
Scene3D::MultiThreadCamera::~MultiThreadCamera(){
	// Delete each thread camera.
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i)
		delete m_pCams[i];
}
Scene3D::Camera *Scene3D::MultiThreadCamera::nextThreadCam(){
	// Round-robin assignment of cameras.
	static int index = 0;
	Camera *c = m_pCams[index ++];

	if(index >= NUM_RENDER_THREADS)
		index = 0;

	return c;
}
void Scene3D::MultiThreadCamera::draw_frame(){
	// Start rendering threads
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		MtCamMem[i]->m_run = true;
	}

	// Draw any meshes which are on the parent camera (not assigned to a thread).
	for(Renderable *mesh : *m_pMeshes)
		mesh->draw_if_cam(0 /* ticks */, this);

	// Wait for threads...
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		while(MtCamMem[i]->m_run) std::this_thread::yield();
	}

	// Merge all thread cameras screenspace_px
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		Camera *c = m_pCams[i];

		// Iterate over PX and ZB, assign PX if ZB is closer.
		for(auto spit = 0; spit < (SCREEN_WIDTH * SCREEN_HEIGHT); ++ spit){
			if(c->screenspace_zb[spit] < screenspace_zb[spit]){
				screenspace_zb[spit] = c->screenspace_zb[spit];

				// Is there a faster copy than this?
				for(int ii = 0; ii < 4; ++ ii){
					screenspace_px[spit * 4 + ii] = c->screenspace_px[spit * 4 + ii];
				}
			}
		}

		c->clear();
	}

	// Copy to our texture and render to the screen.
	Camera::draw_frame();
}
void Scene3D::MultiThreadCamera::cache(){
	Camera::cache();

	// Sync values to all thread cameras before draw.
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		Camera *c = m_pCams[i];

		if(c)
			*c = *this;
	}
}
void mtCamWorker(int n){
	Scene3D::MultiThreadCamera::Shmem *mem = MtCamMem[n];

	while(mem->m_alive){
		std::this_thread::yield();

		if(mem->m_run){
			if(mem->m_pMeshes){
				for(Scene3D::Renderable *mesh : *mem->m_pMeshes)
					mesh->draw_if_cam(0 /* ticks */, mem->m_cam);
			}

			// Done for now.
			mem->m_run = false;
		}
	}

	// Thread execution complete.
	mem->m_done = true;
}

Scene3D::Renderable::Renderable(Scene3D::Camera *cam) :
	Drawable(cam->rend),
	cam(cam)
{}
void Scene3D::Renderable::draw_if_cam(int ticks, Scene3D::Camera const *refCam){
	// Draw if this renderable is assigned this camera.
	if(cam == refCam)
		draw(ticks);
}

Scene3D::Mesh::Face::Face(vector<int> vertIds, const byte_t fill[4]){
	this->vertIds = vertIds;
	this->fill = (byte_t*) calloc(4, sizeof(byte_t));
	memcpy(this->fill, fill, 4);
}
void Scene3D::Mesh::Face::set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a){
	const byte_t color[4] = { b, g, r, a };

	memcpy(fill, color, 4);
}

void Scene3D::Mesh::resetScanlines(){
	for(int line = y_min; line <= y_max; line++)
		scanlines[line] = (pixel){ SCREEN_WIDTH, 0 };

	y_min = SCREEN_HEIGHT - 1;
	y_max = 0;
}
Scene3D::Mesh::Mesh(Scene3D::Camera *cam, vector<Scene3D::coord> vertices, list<Scene3D::Mesh::Face*> faces) :
	Renderable(cam)
{
	this->vertices = vertices;

	for(Face *face : faces){
		face->mesh = this;
		this->faces.push_back(face);
	}

	scanlines = (pixel*) calloc(SCREEN_HEIGHT, sizeof(pixel));
	scanlines_coords = (coord*) calloc(SCREEN_HEIGHT * 2, sizeof(coord));
	y_min = 0;
	y_max = SCREEN_HEIGHT - 1;
	resetScanlines();
}
Scene3D::Mesh::~Mesh(){
	free(scanlines);
	free(scanlines_coords);

	for(Face *f : faces)
		delete f;
}
Scene3D::Mesh* Scene3D::Mesh::load(Scene3D::Camera *cam, string fname){
	// Load mesh data from an asset file.
	FileLoader *fl = FileLoader::get(fname);

	if(!fl)
		return NULL;

	stringstream data(fl->text());
	vector<coord> vertices;
	list<Face*> faces;
	int vert_offset = 0;

	// Match a string identifier opening a curly brace block
	regex rx_mesh_start("\\s*(\\S*)\\s*\\{\\s*");
	regex rx_mesh_end("\\s*\\}\\s*");
	regex rx_braced("\\s*\\{([^\\}]*)\\}(.*)");
	regex rx_spaced_numbers("[0-9.-]+");
	regex rx_rgba(".*([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2}).*");

	// Parse all mesh data from resource.
	string line;
	while(getline(data, line)){
		if(line.length() == 0)
			continue;

		// Out of data to parse.
		if(line == "EOF")
			break;

		smatch sm;
		if(regex_match(line, sm, rx_mesh_start)){
			string mesh_name = sm[1];
			int verts = 0;

			while(getline(data, line)){
				if(line.length() == 0)
					continue;

				// Move up a level.
				if(regex_match(line, rx_mesh_end))
					break;

				// Found a new block element.
				if(regex_match(line, sm, rx_mesh_start)){
					string mesh_element = sm[1];

					bool faces_element = false;
					bool coords_element = false;

					if(mesh_element == "coords")
						coords_element = true;
					else if(mesh_element == "faces")
						faces_element = true;

					while(getline(data, line)){
						if(line.length() == 0)
							continue;

						// Move up a level.
						if(regex_match(line, rx_mesh_end))
							break;

						if(coords_element || faces_element){
							if(regex_match(line, sm, rx_braced)){
								string braced_data = sm[1];
								string braced_data_extra = sm[2];

								if(coords_element){
									coord c;

									for(int i = 0; (i < 3) && regex_search(braced_data, sm, rx_spaced_numbers); i++){
										double pt = atof(sm[0].str().c_str());

										switch(i){
											case 0:
												c.x = pt;
												break;
											case 1:
												c.y = pt;
												break;
											case 2:
												c.z = pt;
												break;
										}

										braced_data = sm.suffix().str();
									}

									vertices.push_back(c);
									verts++;
								}

								if(faces_element){
									vector<int> vertIds;
									byte_t color[4];

									// Get the fill color for this face.
									if(regex_match(braced_data_extra, sm, rx_rgba)){
										for(int i = 0; i < 4; i++){
											unsigned char color_value;
											stringstream color_string;

											color_string << hex << sm[i + 1];
											color_string >> color_value;

											color[i] = color_value;
										}
									}

									while(regex_search(braced_data, sm, rx_spaced_numbers)){
										vertIds.push_back(vert_offset + atoi(sm[0].str().c_str()));
										braced_data = sm.suffix().str();
									}

									if(vertIds.size()){
										Face *face = new Face(vertIds, color);
										faces.push_back(face);
									}
								}
							}
						}
					}

				}

				if(line == "EOF"){
					cout << "Model parsing error in [" << fname << "]" << endl;
					return NULL;
				}
			}

			vert_offset += verts;
		}
	}

	return new Mesh(cam, vertices, faces);
}
void Scene3D::Mesh::translate(const Scene3D::coord &delta){
	for(coord &c : vertices)
		c = c + delta;
}
void Scene3D::Mesh::set_color_fill(const byte_t &r, const byte_t &g, const byte_t &b, const byte_t &a){
	// Set the fill color for all faces of this Mesh.
	for(Face *face : faces)
		face->set_color_fill(r, g, b, a);
}
void Scene3D::Mesh::populateScreenspace(){
	for(int i = 0, len = vertices.size(); i < len; i++)
		vertIdToScreen[i] = cam->vertex_screenspace(vertices[i]);
}
void Scene3D::Mesh::drawLine(const int &vert_a, const int &vert_b){
	// Draw a line on the screen to connect two pixels.
	coord a = vertices[vert_a];
	coord b = vertices[vert_b];

	// Try to ignore verts behind the camera.
	{
		double yaw_a = abs((a - cam->pos).angle_xz() - cam->point_xz);
		double yaw_b = abs((b - cam->pos).angle_xz() - cam->point_xz);

		if(
			((yaw_a > (M_PI / 2)) && (yaw_b > cam->maxangle_w)) ||
			((yaw_b > (M_PI / 2)) && (yaw_a > cam->maxangle_w))
		)
			return;
	}

	double step;
	{
		pixel from = vertIdToScreen[vert_a];
		pixel to = vertIdToScreen[vert_b];

		double dx = to.x - from.x;
		double dy = to.y - from.y;

		// Reducing this step size greatly improves performance;
		step = 1.5 * ((abs(dx) >= abs(dy)) ? abs(dx) : abs(dy));
	}

	coord coord_step = (b - a) / step;

	for(int i = 1; i <= step; i++){
		pixel px = cam->vertex_screenspace(a + (coord_step * i));

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
	}
}
void Scene3D::Mesh::drawFace(const Scene3D::Mesh::Face &face){
	resetScanlines();

	// Draw the border, and build a set of pixel coordinates that
	// represent the outline.
	for(int i = 0, len = face.vertIds.size(); i < len; i++)
		drawLine(face.vertIds[i], face.vertIds[((i == len - 1) ? 0 : (i + 1))]);

	if(y_min < 0)
		y_min = 0;
	if(y_max > (SCREEN_HEIGHT - 1))
		y_max = (SCREEN_HEIGHT - 1);

	// Fill each line.
	if(y_min < y_max){
		for(int line = y_min; line <= y_max; line++){
			// Draw every other line.
			if(cam->m_interlace){
				if(cam->m_oddscanline){
					if((line % 2) == 0)
						continue;
				} else if((line % 2) == 1)
					continue;
			}

			pixel bounds = scanlines[line];
			coord coord_left = scanlines_coords[2 * line];
			coord coord_delta = (scanlines_coords[2 * line + 1] - coord_left) / (bounds.y - bounds.x);

			for(int x = bounds.x; x < bounds.y; x++){
				if((x >= 0) && (line >= 0) && (x < SCREEN_WIDTH) && (line < SCREEN_HEIGHT)){
					const unsigned int offset = (SCREEN_WIDTH * line + x);
					double distance = cam->pos.distance_to(coord_left + (coord_delta * (x - bounds.x)));

					// Draw this pixel if there isn't already one in front of it.
					if(distance < cam->screenspace_zb[offset]){
						memcpy(&cam->screenspace_px[offset * 4], face.fill, 4);
						cam->screenspace_zb[offset] = distance;
					}
				}
			}
		}
	}
}
void Scene3D::Mesh::draw(int ticks){
	populateScreenspace();

	// Sort faces by distance to the camera. Far faces are drawn first.
	for(Face *face : faces)
		drawFace(*face);
}

Scene3D::~Scene3D()
{ }
Scene3D::Scene3D(Controller *ctrl) :
	Scene(ctrl)
{ }
void Scene3D::draw(int ticks){
	// Update camera's cached math results.
	cam->cache();

	// Render meshes and paint the frame to the screen.
	cam->draw_frame();

	// Draw everything else on top.
	Scene::draw(ticks);
}
void Scene3D::check_mouse(SDL_Event event){
	// Check for mouse activity in 3D space.
	// TODO

	// Send mouse events to 2D space.
	Scene::check_mouse(event);
}
