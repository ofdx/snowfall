class TestScene3D : public Scene3D {
	PicoText *text_xyz, *text_pry;

	// FIXME debug
	class TestSaveButton : public Button {
		Camera *cam;

	public:
		TestSaveButton(Camera *cam) :
			Button(cam->rend, 10, 30, 1, 4, string("save"))
		{
			this->cam = cam;
		}

		void action(){
			player_data data = {
				"krak",
				"",
				1, 2,
				cam->pos.x, cam->pos.y, cam->pos.z,
				cam->point.x, cam->point.y, cam->point.z
			};

			player_data_save(&data);
		}
	} *testSaveButton;

	// FIXME debug
	class TestLoadButton : public Button {
		Camera *cam;

	public:
		TestLoadButton(Camera *cam) :
			Button(cam->rend, 10, 50, 1, 4, string("load"))
		{
			this->cam = cam;
		}

		void action(){
			player_data *data = player_data_read();

			if(data){
				cam->pos = (coord){ data->x, data->y, data->z };
				cam->point = (coord){ data->point_x, data->point_y, data->point_z };
			} else {
				cout << "Failed to load player data!" << endl;
			}
		}
	} *testLoadButton;

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

	list<Scene3D::Mesh*> rendered_meshes;

public:
	TestScene3D(Scene::Controller *ctrl) : Scene3D(ctrl) {
		cam = new Camera(rend, { -6.7, 1, 4.6 }, { 1, 0, -1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.46 /* approximately 90 degrees horizontal FOV */);
		clickables.push_back(cam);

		{
			Mesh *mesh = Scene3D::Mesh::load(cam, "models/test_room.mesh");

			drawable_meshes.push_back(mesh);
			rendered_meshes.push_back(mesh);
		}

		{
			Mesh *mesh = Scene3D::Mesh::load(cam, "models/wizard.mesh");

			drawable_meshes.push_back(mesh);
			rendered_meshes.push_back(mesh);
		}

		text_xyz = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 20,
			SCREEN_WIDTH, 10
		}, "");
		drawables.push_back(text_xyz);
		text_xyz->set_color(0xff, 0x0, 0xff);

		text_pry = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 10,
			SCREEN_WIDTH, 10
		}, "");
		drawables.push_back(text_pry);
		text_pry->set_color(0xff, 0x0, 0xff);

		// Camera control buttons
		y_plus = new CameraControlButton(cam, (Scene3D::coord){ 0, 0.1, 0}, (Scene3D::coord){ 0, 0, 0 }, rend, 30, 10, "Y+");
		y_minus = new CameraControlButton(cam, (Scene3D::coord){ 0, -0.1, 0 }, (Scene3D::coord){ 0, 0, 0 }, rend, 10, 10, "Y-");
		drawables.push_back(y_plus);
		clickables.push_back(y_plus);
		drawables.push_back(y_minus);
		clickables.push_back(y_minus);

		// FIXME debug
		testSaveButton = new TestSaveButton(cam);
		drawables.push_back(testSaveButton);
		clickables.push_back(testSaveButton);

		// FIXME debug
		testLoadButton = new TestLoadButton(cam);
		drawables.push_back(testLoadButton);
		clickables.push_back(testLoadButton);
	}

	void draw(int ticks){
		// Keyboard walking
		{
			double walk_speed = ticks / 1000.0 / 5.0;
			coord walk_dir = { 0, 0, 0 };

			// X represents fore/aft movement.
			if(ctrl->keystate(SDLK_w) || ctrl->keystate(SDLK_UP))
				walk_dir.x += 1;

			if(ctrl->keystate(SDLK_s) || ctrl->keystate(SDLK_DOWN))
				walk_dir.x -= 1;

			// Z represents left/right movement.
			if(ctrl->keystate(SDLK_a))
				walk_dir.z += 1;

			if(ctrl->keystate(SDLK_d))
				walk_dir.z -= 1;

			// Walk if we have a direction
			if(!(walk_dir == (coord){ 0, 0, 0})){
				double dir = walk_dir.angle_xz().getValue();

				if(dir)
					cam->yaw(dir);

				cam->walk(ticks * walk_speed);

				if(dir)
					cam->yaw(-dir);
			}
		}

		// Keyboard turning
		{
			double turn_speed = PI * ticks / 1000.0; // 180 degrees per second
			int turn_dir = 0;

			if(ctrl->keystate(SDLK_LEFT))
				turn_dir += 1;

			if(ctrl->keystate(SDLK_RIGHT))
				turn_dir -= 1;

			if(turn_dir)
				cam->yaw(turn_dir * turn_speed);
		}

		// Toggle mouse look. This needs to move into scene3d.h.
		{
			static bool toggle_mlook = false;

			if(ctrl->keystate(SDLK_SPACE)){
				if(!toggle_mlook){
					toggle_mlook = true;
					cam->mlook_toggle();
				}
			} else toggle_mlook = false;
		}

		// Toggle face filling, leaving just a wireframe border around faces.
		{
			static bool toggle_wireframe = false;

			if(ctrl->keystate(SDLK_f)){
				if(!toggle_wireframe){
					toggle_wireframe = true;
					cam->wireframe = !cam->wireframe;
				}
			} else toggle_wireframe = false;
		}

		/* on-screen debug */
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
		/* on-screen debug */


		Scene3D::draw(ticks);
	}

	~TestScene3D(){
		delete text_xyz;
		delete text_pry;

		delete y_plus;
		delete y_minus;

		delete testSaveButton;
		delete testLoadButton;

		for(Scene3D::Mesh *mesh : rendered_meshes)
			delete mesh;

		delete cam;
	}
};
