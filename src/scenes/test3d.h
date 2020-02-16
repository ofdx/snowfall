class TestScene3D : public Scene3D {
	PicoText *text_xyz, *text_pry;

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
	} *y_plus, *y_minus;

	class CameraRotateButton : public Button {
		Camera *cam;
		double delta;

	public:
		CameraRotateButton(Camera *cam, double delta, SDL_Renderer *rend, int x, int y, string text) :
			Button(rend, x, y, 1, text.length(), text)
		{
			this->cam = cam;
			this->delta = delta;
		}

		void action(){
			cam->yaw(delta);
		}
	} *xzr_plus, *xzr_minus;

	class CameraPitchButton : public Button {
		Camera *cam;
		double delta;

	public:
		CameraPitchButton(Camera *cam, double delta, SDL_Renderer *rend, int x, int y, string text) :
			Button(rend, x, y, 1, text.length(), text)
		{
			this->cam = cam;
			this->delta = delta;
		}

		void action(){
			cam->pitch(delta);
		}
	} *yr_plus, *yr_minus;;

	class CameraMoveButton : public Button {
		Camera *cam;
		double dir, walk, fly;

	public:
		CameraMoveButton(Camera *cam, double dir, double walk, double fly, SDL_Renderer *rend, int x, int y, string text) :
			Button(rend, x, y, 1, text.length(), text)
		{
			this->cam = cam;
			this->dir = dir;
			this->walk = walk;
			this->fly = fly;
		}

		void action(){
			if(dir)
				cam->yaw(dir);

			cam->walk(walk);
			// cam->fly(fly);

			if(dir)
				cam->yaw(-dir);
		}
	} *xzw_plus, *xzw_minus, *strafe_right, *strafe_left;

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
	};

	list<Mesh*> rendered_meshes;

public:
	TestScene3D(Scene::Controller *ctrl) : Scene3D(ctrl) {
		cam = new Camera( { -4.25, 1.5, 4 }, { 0, 0, 1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.46 /* approximately 90 degrees horizontal FOV */);

		// Grid
		{
			for(int x = -10; x < 10; x++)
				for(int z = -10; z < 10; z++){
					vector<Scene3D::coord> coords {
						{ x-0.5, 0, z-0.5 },
						{ x+0.5, 0, z-0.5 },
						{ x+0.5, 0, z+0.5 },
						{ x-0.5, 0, z+0.5 }
					};
					list<vector<int>> faces {
						{ 0, 1, 2, 3 }
					};
					Mesh *mesh = new Mesh(rend, cam, coords, faces);
					rendered_meshes.push_back(mesh);
					drawables.push_back(mesh);
				}
		}

		// 3D cube
		{
			vector<Scene3D::coord> cube_coords {
				{ -5, 0, 5 },   // 0
				{ -5, 0, 6 },   // 1
				{ -4, 0, 6 },   // 2
				{ -4, 0, 5 },   // 3
				{ -5, 1, 5 },   // 4
				{ -5, 1, 6 },   // 5
				{ -4, 1, 6 },   // 6
				{ -4, 1, 5 },   // 7

				{ -2, 0.7, 5 }, // 8
				{ -2, 0.7, 6 }, // 9

				{ -2, 0, 5 },   // 10
				{ -2, 0, 6 }    // 11
			};
			list<vector<int>> cube_faces {
				{ 0, 1, 2, 3 },
				{ 0, 1, 5, 4 },
				{ 1, 2, 6, 5 },
				{ 2, 3, 7, 6 },
				{ 0, 3, 7, 4 },
				{ 4, 5, 6, 7 },

				{ 6, 9, 8, 7 },  // Roof
				{ 6, 9, 11, 2 }, // 6-wall
				{ 7, 8, 10, 3 }  // 5-wall

			};
			Mesh *cube = new Mesh(rend, cam, cube_coords, cube_faces);
			rendered_meshes.push_back(cube);
			drawables.push_back(cube);
		}

		// Witch's Hat
		{
			vector<Scene3D::coord> coords {
				{ -5, 1, 5 },      // 0
				{ -5, 1, 6 },      // 1
				{ -4, 1, 6 },      // 2
				{ -4, 1, 5 },      // 3
				{ -4.5, 2.5, 5.5 } // 4
			};
			list<vector<int>> faces {
				{ 0, 1, 2, 3 }, // Base
				{ 4, 0, 1 },
				{ 4, 1, 2 },
				{ 4, 2, 3 },
				{ 4, 3, 0 }
			};
			Mesh *hat = new Mesh(rend, cam, coords, faces);
			rendered_meshes.push_back(hat);
			drawables.push_back(hat);
		}

		text_xyz = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 20,
			SCREEN_WIDTH, 10
		}, "");
		drawables.push_back(text_xyz);

		text_pry = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 10,
			SCREEN_WIDTH, 10
		}, "");
		drawables.push_back(text_pry);

		// Camera control buttons
		strafe_right = new CameraMoveButton(cam, -(PI / 2), 0.1, 0, rend, 30, 10, "SR");
		strafe_left = new CameraMoveButton(cam, (PI / 2), 0.1, 0, rend, 10, 10, "SL");
		drawables.push_back(strafe_right);
		clickables.push_back(strafe_right);
		drawables.push_back(strafe_left);
		clickables.push_back(strafe_left);

		y_plus = new CameraControlButton(cam, (Scene3D::coord){ 0, 0.1, 0}, (Scene3D::coord){ 0, 0, 0 }, rend, 30, 30, "Y+");
		y_minus = new CameraControlButton(cam, (Scene3D::coord){ 0, -0.1, 0 }, (Scene3D::coord){ 0, 0, 0 }, rend, 10, 30, "Y-");
		drawables.push_back(y_plus);
		clickables.push_back(y_plus);
		drawables.push_back(y_minus);
		clickables.push_back(y_minus);

		xzw_plus = new CameraMoveButton(cam, 0, 0.1, 0, rend, 30, 50, "FW");
		xzw_minus = new CameraMoveButton(cam, 0, -0.1, 0, rend, 10, 50, "BK");
		drawables.push_back(xzw_plus);
		clickables.push_back(xzw_plus);
		drawables.push_back(xzw_minus);
		clickables.push_back(xzw_minus);

		xzr_plus = new CameraRotateButton(cam, PI / 20, rend, 10, 70, "<-");
		xzr_minus = new CameraRotateButton(cam, -(PI / 20), rend, 30, 70, "->");
		drawables.push_back(xzr_plus);
		clickables.push_back(xzr_plus);
		drawables.push_back(xzr_minus);
		clickables.push_back(xzr_minus);

		yr_plus = new CameraPitchButton(cam, PI / 20, rend, 30, 90, "UP");
		yr_minus = new CameraPitchButton(cam, -(PI / 20), rend, 10, 90, "DN");
		drawables.push_back(yr_plus);
		clickables.push_back(yr_plus);
		drawables.push_back(yr_minus);
		clickables.push_back(yr_minus);
	}

	void draw(int ticks){
		text_xyz->set_message("c_pos: " + cam->pos.display());
		text_pry->set_message("c_pnt: " + cam->point.display());

		Scene3D::draw(ticks);
	}

	~TestScene3D(){
		delete text_xyz;
		delete text_pry;

		delete strafe_right;
		delete strafe_left;
		delete y_plus;
		delete y_minus;
		delete xzw_plus;
		delete xzw_minus;
		delete yr_plus;
		delete yr_minus;
		delete xzr_plus;
		delete xzr_minus;

		for(Mesh *mesh : rendered_meshes)
			delete mesh;

		delete cam;
	}
};
