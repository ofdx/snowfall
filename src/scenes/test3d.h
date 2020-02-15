class TestScene3D : public Scene3D {
	// 2D
	class LivingRoomButton : public Button {
		Scene::Controller *ctrl;

	public:
		LivingRoomButton(Scene::Controller *ctrl, SDL_Renderer *rend, int x, int y, string text) : Button(rend, x, y, 1, text.length(), text) {
			this->ctrl = ctrl;
		}

		void action(){
			ctrl->set_scene(Scene::create(ctrl, "living"));
		}

	} *livingRoomButton;

	class CameraControlButton : public Button {
		Camera *cam;
		Scene3D::coord xform_pos, xform_point;

	public:
		CameraControlButton(Camera *cam, Scene3D::coord xform_pos, Scene3D::coord xform_point, SDL_Renderer *rend, int x, int y, string text) :
			Button(rend, x, y, 1, text.length(), text)
		{
			this->cam = cam;
			this->xform_pos = xform_pos;
			this->xform_point = xform_point;
		}

		void action(){
			cam->pos += xform_pos;
			cam->point += xform_point;
		}
	} *x_plus, *x_minus, *y_plus, *y_minus, *z_plus, *z_minus;

	// 3D
	Camera *cam;

	class Mesh : public Renderable {
		vector<Scene3D::coord> vertices;
		list<vector<int>> faces;

	public:
		Mesh(SDL_Renderer *rend, Camera *cam, vector<coord> vertices, list<vector<int>> faces) : Renderable(rend, cam) {
			this->vertices = vertices;
			this->faces = faces;
		}

		virtual void draw(int ticks){
			// Draw faces
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);

			for(auto face : faces){
				for(int i = 0, len = face.size(); i< len; i++){
					pixel px_a = cam->vertex_screenspace(vertices[face[i]]);
					pixel px_b = cam->vertex_screenspace(vertices[face[((i == len - 1) ? 0 : (i + 1))]]);

					if(cam->pixel_visible(px_a) && cam->pixel_visible(px_b))
						cam->drawLine(rend, px_a, px_b);
				}
			}

			// Draw vertices
			SDL_SetRenderDrawColor(rend, 0, 0xff, 0xff, 0xff);

			for(coord vert : vertices){
				pixel px = cam->vertex_screenspace(vert);

				if(cam->pixel_visible(px))
					SDL_RenderDrawPoint(rend, px.x, px.y);
			}
		}
	} *cube;

public:
	TestScene3D(Scene::Controller *ctrl) : Scene3D(ctrl) {
		cam = new Camera( { -4.25, 1.5, 4 }, { 0, 0, 1 }, SCREEN_WIDTH, SCREEN_HEIGHT, PI / 4);

		x_plus = new CameraControlButton(cam, (Scene3D::coord){ 0.1, 0, 0 }, (Scene3D::coord){ 0, 0, 0 }, rend, 30, 10, "X+");
		x_minus = new CameraControlButton(cam, (Scene3D::coord){ -0.1, 0, 0 }, (Scene3D::coord){ 0, 0, 0 }, rend, 10, 10, "X-");
		drawables.push_back(x_plus);
		clickables.push_back(x_plus);
		drawables.push_back(x_minus);
		clickables.push_back(x_minus);

		y_plus = new CameraControlButton(cam, (Scene3D::coord){ 0, 0.1, 0}, (Scene3D::coord){ 0, 0, 0 }, rend, 30, 30, "Y+");
		y_minus = new CameraControlButton(cam, (Scene3D::coord){ 0, -0.1, 0 }, (Scene3D::coord){ 0, 0, 0 }, rend, 10, 30, "Y-");
		drawables.push_back(y_plus);
		clickables.push_back(y_plus);
		drawables.push_back(y_minus);
		clickables.push_back(y_minus);

		z_plus = new CameraControlButton(cam, (Scene3D::coord){ 0, 0, 0.1 }, (Scene3D::coord){ 0, 0, 0 }, rend, 30, 50, "Z+");
		z_minus = new CameraControlButton(cam, (Scene3D::coord){ 0, 0, -0.1 }, (Scene3D::coord){ 0, 0, 0 }, rend, 10, 50, "Z-");
		drawables.push_back(z_plus);
		clickables.push_back(z_plus);
		drawables.push_back(z_minus);
		clickables.push_back(z_minus);


		vector<Scene3D::coord> cube_coords {
			{ -5, 0, 5 },
			{ -5, 0, 6 },
			{ -4, 0, 6 },
			{ -4, 0, 5 },
			{ -5, 1, 5 },
			{ -5, 1, 6 },
			{ -4, 1, 6 },
			{ -4, 1, 5 }
		};
		list<vector<int>> cube_faces {
			{ 0, 1, 2, 3 },
			{ 0, 1, 5, 4 },
			{ 1, 2, 6, 5 },
			{ 2, 3, 7, 6 },
			{ 0, 3, 7, 4 },
			{ 4, 5, 6, 7 }
		};
		cube = new Mesh(rend, cam, cube_coords, cube_faces);

		drawables.push_back(cube);
	}

	~TestScene3D(){
		delete livingRoomButton;
		delete cube;
		delete cam;
	}
};
