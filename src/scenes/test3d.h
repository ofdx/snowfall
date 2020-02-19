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

	list<Scene3D::Mesh*> rendered_meshes;

public:
	TestScene3D(Scene::Controller *ctrl) : Scene3D(ctrl) {
		cam = new Camera( { -6.7, 0.7, 4.6 }, { 1, 0, -1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.46 /* approximately 90 degrees horizontal FOV */);

		// Grid
		{
			for(double x = -10; x <= 10; x++)
				for(double z = -10; z <= 10; z++){
					vector<Scene3D::coord> coords {
						{ x,   0, z   },
						{ x+1, 0, z   },
						{ x+1, 0, z+1 },
						{ x,   0, z+1 }
					};
					list<vector<int>> faces {
						{ 0, 1, 2, 3 }
					};
					Scene3D::Mesh *mesh = new Scene3D::Mesh(rend, cam, coords, faces);
					rendered_meshes.push_back(mesh);
					drawables.push_back(mesh);
				}
		}

		// Some kinda church
		{
			vector<Scene3D::coord> coords {
				{ 0, 0, 0 },   // 0
				{ 0, 0, 1 },   // 1
				{ 1, 0, 1 },   // 2
				{ 1, 0, 0 },   // 3
				{ 0, 1, 0 },   // 4
				{ 0, 1, 1 },   // 5
				{ 1, 1, 1 },   // 6
				{ 1, 1, 0 },   // 7

				{ 3, 0.7, 0 }, // 8
				{ 3, 0.7, 1 }, // 9

				{ 3, 0, 0 },   // 10
				{ 3, 0, 1 },   // 11

				{ 0.5, 2.5, 0.5 } // 4 - 12
			};
			list<vector<int>> faces {
				{ 0, 1, 2, 3 },
				{ 0, 1, 5, 4 },
				{ 1, 2, 6, 5 },
				{ 2, 3, 7, 6 },
				{ 0, 3, 7, 4 },
				{ 4, 5, 6, 7 },

				{ 6, 9, 8, 7 },  // Roof
				{ 6, 9, 11, 2 }, // 6-wall
				{ 7, 8, 10, 3 }, // 5-wall


				// Witch's Hat
				{ 4, 5, 6, 7 }, // Base
				{ 12, 4, 5 },
				{ 12, 5, 6 },
				{ 12, 6, 7 },
				{ 12, 7, 4 }
			};

			Scene3D::Mesh *obj = new Scene3D::Mesh(rend, cam, coords, faces);
			obj->translate((coord){ -5, 0, -5 });

			rendered_meshes.push_back(obj);
			drawables.push_back(obj);
		}

		// Doodad
		{
			vector<Scene3D::coord> coords {
				{ 2, 0, 2 },       // 0
				{-2, 0, 2 },       // 1
				{-2, 0,-2 },       // 2
				{ 2, 0,-2 },       // 3

				{ 1.5, 2, 1.5 },   // 4
				{-1.5, 2, 1.5 },   // 5
				{-1.5, 2,-1.5 },   // 6
				{ 1.5, 2,-1.5 },   // 7

				{ 0, 4, 0 }        // 8
			};
			list<vector<int>> faces {
				{ 0, 1, 2, 3 }, // Base

				// Walls
				{ 0, 4, 8, 5, 1 },
				{ 1, 5, 8, 6, 2 },
				{ 2, 6, 8, 7, 3 },
				{ 3, 7, 8, 4, 0 },

				// Mid-level ring
				{ 4, 5, 6, 7 }
			};

			Scene3D::Mesh *obj = new Scene3D::Mesh(rend, cam, coords, faces);
			obj->translate((coord){ 5, 0, -2 } );

			rendered_meshes.push_back(obj);
			drawables.push_back(obj);
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
		stringstream pry;
		Scene3D::Radian
			rpy(atan2(cam->point.y, sqrt(cam->point.z * cam->point.z + cam->point.x * cam->point.x))),
			rpxz(atan2(cam->point.z, cam->point.x));

		pry
			<< "c_pnt: ("
			<< RAD_TO_DEG(rpxz.getValue()) << ", "
			<< RAD_TO_DEG(rpy.getValue()) << ")";

		text_xyz->set_message("c_pos: " + cam->pos.display());
		text_pry->set_message(pry.str());
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

		for(Scene3D::Mesh *mesh : rendered_meshes)
			delete mesh;

		delete cam;
	}
};
