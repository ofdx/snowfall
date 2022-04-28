class TestScene3D : public Scene3D {
	PicoText *text_xyz, *text_pry, *text_fps;
	PicoText *text_debug;

	bool m_show_cam;
	bool m_console_pending;
	bool m_nightmode;

	//list<Scene3D::Mesh*> rendered_meshes;
	WedgeTerrain *terrain;

public:
	TestScene3D(Scene::Controller *ctrl) :
		Scene3D(ctrl),

		m_show_cam(false),
		m_console_pending(false),
		m_nightmode(false)
	{
		cam = new Camera(rend, { 35, 10, 35 }, { 1, 0, -1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.46 /* approximately 90 degrees horizontal FOV */);
		cam->pitch(-PI / 12);
		cam->yaw(-PI / 2);
		clickables.push_back(cam);

		terrain = new WedgeTerrain(cam);
		drawable_meshes.push_back(terrain);

		/*{
			Mesh *mesh = Scene3D::Mesh::load(cam, "models/pentatower.mesh");

			if(!mesh){
				cout << "Failed to load model!" << endl;
			}

			drawable_meshes.push_back(mesh);
		}*/

		text_debug = new PicoText(rend, (SDL_Rect){
			5, SCREEN_HEIGHT - 30,
			SCREEN_WIDTH, 30
		}, "");
		text_debug->set_color(0xff, 0x0, 0xff);
		if(m_show_cam)
			drawables.push_back(text_debug);
	}

	void receive_data(string data) override {
		if(data.empty())
			return;

		// Immediately return to the console to display status and accept the next command.
		m_console_pending = true;

		stringstream ss(data);
		string verb;

		ss >> verb;
		if(ss){
			if(verb == "quit"){
				ctrl->quit();
			} else if(verb == "close"){
				m_console_pending = false;
			} else if(verb == "state"){
				string action;
				ss >> action;

				if(ss){
					if(action == "load"){
						player_data *data = player_data_read();

						if(data){
							cam->pos = (coord){ data->x, data->y, data->z };
							cam->point = (coord){ data->point_x, data->point_y, data->point_z };
						} else {
							// FIXME debug
							cout << "Failed to load player data!" << endl;

							// TODO - return error to console
						}
					} else if(action == "save"){
						player_data data = {
							"krak",
							"",
							1, 2,
							cam->pos.x, cam->pos.y, cam->pos.z,
							cam->point.x, cam->point.y, cam->point.z
						};

						player_data_save(&data);

						// TODO - status to console if possible
					}
				}
			} else if(verb == "set"){
				string what;
				ss >> what;

				if(ss){
					// Toggle camera debug information (pos, point, fps)
					if(what == "cam"){
						int val;
						ss >> val;

						if(ss){
							if(!m_show_cam && val){
								m_show_cam = true;
								drawables.push_back(text_debug);
							} else if(m_show_cam && !val){
								m_show_cam = false;
								drawables.remove(text_debug);
							}
						}
					}

					// Toggle the effect of night.
					if(what == "night"){
						int val;
						ss >> val;

						if(ss)
							m_nightmode = !!val;
					}

					// Toggle interlace/alternating scanline 3D render mode.
					if(what == "interlace"){
						int val;
						ss >> val;

						if(ss)
							cam->m_interlace = !!val;
					}

					if(what == "fullscreen"){
						bool val;
						ss >> val;

						if(ss && (val != ctrl->fullscreen)){
							ctrl->fullscreen = val;

							SDL_SetWindowFullscreen(ctrl->win, (
								ctrl->fullscreen ?
									SDL_WINDOW_FULLSCREEN_DESKTOP :
									0
							));

							SDL_Delay(500);
						}
					}
				}
			}
		}
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

			if(ctrl->keystate(SDLK_LSHIFT))
				walk_speed *= 2;

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

			if(ctrl->keystate(SDLK_e))
				risefall += 1.0;

			if(ctrl->keystate(SDLK_q))
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

		// Text command entry mode
		{
			static bool toggle_console = false;

			if(ctrl->keystate(SDLK_BACKQUOTE)){
				if(!toggle_console){
					toggle_console = true;
					m_console_pending = true;
				}
			} else if(!m_console_pending)
				toggle_console = false;
		}

		/* on-screen debug */
		if(m_show_cam){
			static int fps_samples[60], fps_pos = 0;

			stringstream ss;

			Scene3D::Radian
				rpy(atan2(cam->point.y, sqrt(cam->point.z * cam->point.z + cam->point.x * cam->point.x))),
				rpxz(atan2(cam->point.z, cam->point.x));

			ss << "c_pos: " << cam->pos.display() << endl;
			ss << "c_pnt: (" << RAD_TO_DEG(rpxz.getValue()) << ", " << RAD_TO_DEG(rpy.getValue()) << ")" << endl;

			if(ticks){
				fps_samples[fps_pos++] = (1000.0 / ticks);

				if(fps_pos >= 60)
					fps_pos = 0;
			}

			int fps = 0;

			for(int i = 0; i < 60; i++){
				fps += fps_samples[i];
			}

			ss << "  fps: " << (fps / 60);

			text_debug->set_message(ss.str());
		}
		/* on-screen debug */

		Scene3D::draw(ticks);

		if(m_nightmode){
			SDL_Rect fsrect = (SDL_Rect){
				0, 0,
				SCREEN_WIDTH, SCREEN_HEIGHT
			};

			SDL_SetRenderDrawColor(rend, 0x30, 0, 0x60, 0x50);
			SDL_RenderFillRect(rend, &fsrect);
		}

		if(m_console_pending){
			// Disable mouse look so that the mouse cursor will be available in the dialog.
			cam->mlook(false);

			m_console_pending = false;
			ctrl->scene_descend(Scene::create(ctrl, "modal_input_text"));
		}
	}

	~TestScene3D(){
		delete text_xyz;
		delete text_pry;
		delete text_fps;

		//for(Scene3D::Mesh *mesh : rendered_meshes)
		//	delete mesh;

		delete terrain;

		delete cam;
	}
};
