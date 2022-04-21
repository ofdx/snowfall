class TestScene3D : public Scene3D {
	PicoText *text_xyz, *text_pry, *text_fps;

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

	//list<Scene3D::Mesh*> rendered_meshes;
	WedgeTerrain *terrain;

public:
	TestScene3D(Scene::Controller *ctrl) : Scene3D(ctrl) {
		cam = new Camera(rend, { -6.7, 1, 4.6 }, { 1, 0, -1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.46 /* approximately 90 degrees horizontal FOV */);
		clickables.push_back(cam);

		terrain = new WedgeTerrain(cam);
		drawable_meshes.push_back(terrain);

		/*{
			Mesh *mesh = Scene3D::Mesh::load(cam, "models/pentatower.mesh");

			if(!mesh){
				cout << "Failed to load model!" << endl;
			}

			drawable_meshes.push_back(mesh);
			rendered_meshes.push_back(mesh);
		}*/

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

		text_fps = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 30,
			SCREEN_WIDTH, 10
		}, "");
		drawables.push_back(text_fps);
		text_fps->set_color(0xff, 0x0, 0xff);

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
		double walk_speed = ticks / 1000.0 / 5.0;

		// Keyboard walking
		{
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

		// Rise and fall.
		{
			double risefall = 0.0;

			if(ctrl->keystate(SDLK_q))
				risefall += 1.0;

			if(ctrl->keystate(SDLK_e))
				risefall -= 1.0;

			if(risefall){
				risefall *= (ticks * walk_speed);

				cam->pos.y += risefall;
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

		// Toggle interlaced 3D rendering.
		{
			static bool toggle_interlace = false;

			if(ctrl->keystate(SDLK_i)){
				if(!toggle_interlace){
					toggle_interlace = true;
					cam->m_interlace = !cam->m_interlace;
				}
			} else toggle_interlace = false;
		}

		/* on-screen debug */
		{
			if(ticks)
				text_fps->set_message(string("  fps: ") + to_string(1000.0 / ticks));

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
		}
		/* on-screen debug */

		Scene3D::draw(ticks);
	}

	~TestScene3D(){
		delete text_xyz;
		delete text_pry;
		delete text_fps;

		delete testSaveButton;
		delete testLoadButton;

		//for(Scene3D::Mesh *mesh : rendered_meshes)
		//	delete mesh;

		delete terrain;

		delete cam;
	}
};
