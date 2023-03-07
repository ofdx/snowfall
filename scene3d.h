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
		double m_value;

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
		bool m_mlook_active = false;

	public:
		coord m_pos, m_point;
		Radian m_point_xz = 0, m_point_y = 0;

		double m_maxangle_w, m_maxangle_h;
		int m_w, m_h;

		byte_t m_screenspace_px[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
		double m_screenspace_zb[SCREEN_WIDTH * SCREEN_HEIGHT];
		SDL_Texture *m_screenspace_tx;
		SDL_Renderer *m_rend;

		double m_max_pitch;

		bool m_oddscanline, m_interlace;
		bool m_wireframe;

		Camera(SDL_Renderer *rend, coord pos, coord point, int w, int h, double maxangle);
		Camera operator = (Camera const& other);
		virtual ~Camera();

		void set_fov(double const maxangle);
		double get_fov() const;

		pixel vertex_screenspace(const coord &vertex) const;
		void yaw(double const& delta);
		void pitch(double const& delta);
		void walk(double const& distance);
		void mlook(bool const active);
		bool mlook_toggle();
		virtual void check_mouse(SDL_Event event) override;
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
			std::atomic<bool> m_alive, m_run, m_done, m_clear;
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
	m_value = Normalize(value);
}
double Scene3D::Radian::operator + (const double &d) const {
	return m_value + d;
}
double Scene3D::Radian::operator - (const Radian &other) const {
	double reverse = Normalize(M_PI + other.m_value);
	bool left = false;

	if(reverse > other.m_value){
		// Left is on the outside.
		if((m_value <= other.m_value) || (m_value > reverse))
			left = true;
	} else {
		// Left is on the inside.
		if((m_value <= other.m_value) && (m_value > reverse))
			left = true;
	}

	if(left){
		if(m_value > other.m_value)
			return -(other.m_value + (2 * M_PI) - m_value);

		return -(other.m_value - m_value);
	}

	if(m_value < other.m_value)
		return ((2 * M_PI) - other.m_value + m_value);

	return (m_value - other.m_value);
}
inline double Scene3D::Radian::getValue() const {
	return m_value;
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
	m_pos(pos),
	m_point(point),
	m_w(w),
	m_h(h),
	m_rend(rend),
	m_max_pitch(MAX_CAM_PITCH),
	m_oddscanline(false), m_interlace(false),
	m_wireframe(false)
{
	set_fov(maxangle);

	memset((void*) m_screenspace_px, 0, (sizeof(byte_t) * SCREEN_WIDTH * SCREEN_HEIGHT * 4));
	memset((void*) m_screenspace_zb, MAX_DRAW_DISTANCE, (sizeof(double) * SCREEN_WIDTH * SCREEN_HEIGHT));
	m_screenspace_tx = SDL_CreateTexture(m_rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	cache();
}
Scene3D::Camera Scene3D::Camera::operator = (Scene3D::Camera const& other){
	m_mlook_active = other.m_mlook_active;
	m_pos = other.m_pos;
	m_point = other.m_point;
	m_point_xz = other.m_point_xz;
	m_point_y = other.m_point_y;
	m_maxangle_w = other.m_maxangle_w;
	m_maxangle_h = other.m_maxangle_h;
	m_w = other.m_w;
	m_h = other.m_h;
	m_max_pitch = other.m_max_pitch;
	m_oddscanline = other.m_oddscanline;
	m_interlace = other.m_interlace;
	m_wireframe = other.m_wireframe;

	return *this;
}
Scene3D::Camera::~Camera(){
	SDL_DestroyTexture(m_screenspace_tx);
}
void Scene3D::Camera::set_fov(double const maxangle){
	m_maxangle_w = m_maxangle_h = maxangle;

	// Give whichever direction is smaller a lesser FOV.
	if(m_w > m_h){
		m_maxangle_h = m_maxangle_h / m_w * m_h;
	} else if (m_h > m_w){
		m_maxangle_w = m_maxangle_w / m_h * m_w;
	}
}
double Scene3D::Camera::get_fov() const {
	return ((m_maxangle_w > m_maxangle_h) ? m_maxangle_w : m_maxangle_h);
}
Scene3D::pixel Scene3D::Camera::vertex_screenspace(const Scene3D::coord &vertex) const {
	// Get the x,y coordinates of a pixel on screen to represent this visible vertex.
	coord rel = vertex - m_pos;

	double yaw = (rel.angle_xz() - m_point_xz);
	double pitch = (rel.angle_y() - m_point_y);

	return (pixel){
		x: m_w - (int)((m_w / 2) + (yaw / (2 * m_maxangle_w) * m_w)),
		y: m_h - (int)((m_h / 2) + (pitch / (2 * m_maxangle_h) * m_h))
	};
}
void Scene3D::Camera::yaw(double const& delta){
	// Turn the camera the specified number of radians around the Y-axis.
	double const xz = m_point_xz + delta;

	m_point = (coord){ cos(xz), m_point.y, sin(xz) };
	cache();
}
void Scene3D::Camera::pitch(double const& delta){
	// Pitch the camera up or down the specified number of radians.
	double y = m_point_y + delta;

	if(y < M_PI){
		if(y > m_max_pitch)
			y = m_max_pitch;
	} else {
		if(y < ((2 * M_PI) - m_max_pitch))
			y = ((2 * M_PI) - m_max_pitch);
	}

	m_point.y = sin(y) * sqrt(SQUARE(m_point.x) + SQUARE(m_point.y) + SQUARE(m_point.z));
	cache();
}
void Scene3D::Camera::walk(double const& distance){
	double heading = m_point_xz.getValue();

	m_pos += (coord){ distance * cos(heading), 0, distance * sin(heading) };
	cache();
}
void Scene3D::Camera::mlook(bool const active){
	if(m_mlook_active != active)
		SDL_SetRelativeMouseMode(active ? SDL_TRUE : SDL_FALSE);

	m_mlook_active = active;
}
bool Scene3D::Camera::mlook_toggle(){
	mlook(!m_mlook_active);

	return m_mlook_active;
}
void Scene3D::Camera::check_mouse(SDL_Event event){
	switch(event.type){
		case SDL_MOUSEMOTION:
			if(m_mlook_active){
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
	byte_t *r = nullptr;

	for(int i = 0; i < SCREEN_HEIGHT; ++ i){
		if(m_interlace){
			int const linemod = (i % 2);

			if(!m_oddscanline){
				if(!linemod){
					continue;
				}
			} else if(linemod){
				continue;
			}
		}

		// Clear z-buffer for this line.
		memset(&m_screenspace_zb[i * SCREEN_WIDTH], MAX_DRAW_DISTANCE, sizeof(double) * SCREEN_WIDTH);

		// Clear screen on this line.
		if(!r){
			// Set the entire screen buffer to a background color.
			byte_t const fill[4] = { 0x10, 0x29, 0xad, 0xff }; // BGRA

			// Store a pointer to this line to copy from later.
			r = &m_screenspace_px[i * SCREEN_WIDTH * 4];

			for(int col = 0; col < SCREEN_WIDTH; ++ col)
				memcpy(&r[col * 4], fill, 4);
		} else memcpy(&m_screenspace_px[i * SCREEN_WIDTH * 4], r, (SCREEN_WIDTH * 4));
	}

	m_oddscanline = !m_oddscanline;
}
void Scene3D::Camera::draw_frame(){
	// Update the screen texture and draw it.
	SDL_UpdateTexture(m_screenspace_tx, NULL, &m_screenspace_px[0], SCREEN_WIDTH * 4);
	SDL_RenderCopy(m_rend, m_screenspace_tx, NULL, NULL);

	// Clear everything to prepare for the next frame.
	clear();
}
void Scene3D::Camera::cache(){
	m_point_xz = m_point.angle_xz();
	m_point_y = m_point.angle_y();
}


// Shared memory for camera threads.
Scene3D::MultiThreadCamera::Shmem *MtCamMem[NUM_RENDER_THREADS] = {};
Scene3D::MultiThreadCamera::Shmem::Shmem(list<Scene3D::Renderable*> *pMeshes, Scene3D::Camera *cam) :
	m_alive(true),
	m_run(false),
	m_done(false),
	m_clear(false),
	m_cam(cam),
	m_pMeshes(pMeshes)
{ }

Scene3D::MultiThreadCamera::MultiThreadCamera(SDL_Renderer *rend, Scene3D::coord pos, Scene3D::coord point, int w, int h, double maxangle, list<Scene3D::Renderable*> *pMeshes) :
	Scene3D::Camera(rend, pos, point, w, h, maxangle),
	m_pMeshes(pMeshes)
{
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		// Create camera and its shared memory.
		MtCamMem[i] = new Shmem(m_pMeshes, (m_pCams[i] = new Camera(m_rend, m_pos, m_point, m_w, m_h, maxangle)));

		// Create camera worker thread.
		std::thread worker(mtCamWorker, i);
		worker.detach();
	}
}
Scene3D::MultiThreadCamera::~MultiThreadCamera(){
	// Camera thread cleanup.
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		Shmem *mem = MtCamMem[i];

		// Ask the thread to terminate.
		mem->m_alive = false;
		while(!mem->m_done) std::this_thread::yield();

		// Clean up memory.
		delete m_pCams[i];
		delete mem;
	}
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
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i)
		MtCamMem[i]->m_run = true;

	// Draw any meshes which are on the parent camera (not assigned to a thread).
	for(Renderable *mesh : *m_pMeshes)
		mesh->draw_if_cam(0 /* ticks */, this);

	// Merge all thread cameras screenspace_px
	for(auto i = 0; i < NUM_RENDER_THREADS; ++ i){
		MultiThreadCamera::Shmem *mem = MtCamMem[i];
		Camera const *c = mem->m_cam;

		// Wait for thread to complete drawing.
		while(mem->m_run) std::this_thread::yield();

		// Iterate over PX and ZB, assign PX if ZB is closer.
		for(auto spit = 0; spit < (SCREEN_WIDTH * SCREEN_HEIGHT); ++ spit){
			if(c->m_screenspace_zb[spit] < m_screenspace_zb[spit]){
				memcpy(&m_screenspace_px[spit * 4], &c->m_screenspace_px[spit * 4], 4);
				m_screenspace_zb[spit] = c->m_screenspace_zb[spit];
			}
		}

		// Have the render thread clear its own camera buffer.
		mem->m_clear = true;
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

		// Clear the last frame.
		if(mem->m_clear){
			mem->m_cam->clear();
			mem->m_clear = false;
		}

		// Draw this frame.
		if(mem->m_run){
			for(Scene3D::Renderable *mesh : *mem->m_pMeshes)
				mesh->draw_if_cam(0 /* ticks */, mem->m_cam);

			mem->m_run = false;
		}
	}

	// Thread execution complete.
	mem->m_done = true;
}

Scene3D::Renderable::Renderable(Scene3D::Camera *cam) :
	Drawable(cam->m_rend),
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
		double yaw_a = abs((a - cam->m_pos).angle_xz() - cam->m_point_xz);
		double yaw_b = abs((b - cam->m_pos).angle_xz() - cam->m_point_xz);

		if(
			((yaw_a > (M_PI / 2)) && (yaw_b > cam->m_maxangle_w)) ||
			((yaw_b > (M_PI / 2)) && (yaw_a > cam->m_maxangle_w))
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

		// Paint wireframe
		if(cam->m_wireframe){
			if((px.x >= 0) && (px.y >= 0) && (px.x < SCREEN_WIDTH) && (px.y < SCREEN_HEIGHT)){
				unsigned int const offset = (SCREEN_WIDTH * px.y + px.x);
				const byte_t color[4] = { 0, 0, 0, 0xff };

				memcpy(&cam->m_screenspace_px[offset * 4], color, 4);
				cam->m_screenspace_zb[offset] = 0; /* always on top */
			}
		}

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
					double distance = cam->m_pos.distance_to(coord_left + (coord_delta * (x - bounds.x)));

					// Draw this pixel if there isn't already one in front of it.
					if(distance < cam->m_screenspace_zb[offset]){
						memcpy(&cam->m_screenspace_px[offset * 4], face.fill, 4);
						cam->m_screenspace_zb[offset] = distance;
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
