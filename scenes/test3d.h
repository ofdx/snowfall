class TestScene3D : public Scene3D {
	DebugConsole m_console;

	PicoText *text_xyz, *text_pry, *text_fps;
	PicoText *text_debug;

	bool m_show_cam;
	bool m_console_pending;
	bool m_nightmode, m_nightclock;

	SDL_Color m_night_tint;

	//list<Scene3D::Mesh*> rendered_meshes;
	WedgeTerrain *terrain;

	int m_tickcounter;

public:
	TestScene3D(Scene::Controller *ctrl) :
		Scene3D(ctrl),

		m_show_cam(false),
		m_console_pending(false),
		m_nightmode(false),
		m_nightclock(false),
		m_night_tint((SDL_Color){ 0x30, 0, 0x60, 0x50}),
		m_tickcounter(0)
	{
		cam = new MultiThreadCamera(rend, { 35, 10, 35 }, { -1, -0.5, -1 }, SCREEN_WIDTH, SCREEN_HEIGHT, 0.66 /* seems nice if the cam is 2+ units above ground */, &drawable_meshes);
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

		stringstream ss(data), ss_log;
		bool valid = true;

		string verb;
		ss >> verb;

		if(ss){
			// Start building the log message which will appear in the scrollback.
			ss_log << verb;

			if(verb == "quit"){
				ctrl->quit();
			} else if(verb == "!harvest"){
				ss_log << " is not a command.";
			} else if(verb == "clear"){
				ss_log.str("");
				m_console.log_clear();
			} else if(verb == "state"){
				string action;
				ss >> action;

				if(ss){
					ss_log << " " << action;

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
					} else valid = false;
				} else valid = false;
			} else if(verb == "set"){
				string what;
				ss >> what;

				if(ss){
					ss_log << " " << what;

					// Toggle camera debug information (pos, point, fps)
					if(what == "cam"){

						string item;
						ss >> item;

						if(ss){
							ss_log << " " << item;

							if(item == "info"){
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

								ss_log << " " << m_show_cam;
							} else if(item == "pos"){
								double x, y, z;
								ss >> x >> y >> z;

								if(ss)
									cam->pos = (coord){ .x = x, .y = y, .z = z };

								ss_log << " " << cam->pos.x << " " << cam->pos.y << " " << cam->pos.z;
							} else if(item == "point"){
								double x, y, z;
								ss >> x >> y >> z;

								if(ss)
									cam->point = (coord){ .x = x, .y = y, .z = z };

								ss_log << " " << cam->point.x << " " << cam->point.y << " " << cam->point.z;
							} else if(item == "pitch-max"){
								double angle;
								ss >> angle;

								if(ss)
									cam->max_pitch = abs(angle);

								ss_log << " " << cam->max_pitch;
							} else if(item == "fov"){
								double fov;
								ss >> fov;

								if(ss)
									cam->set_fov(fov);

								ss_log << " " << cam->get_fov();
							} else valid = false;
						} else valid = false;
					}

					// Toggle the effect of night.
					else if(what == "night"){
						int val;
						ss >> val;

						if(ss)
							m_nightmode = !!val;

						ss_log << " " << m_nightmode;
					}
					else if(what == "night-clock"){
						int val;
						ss >> val;

						if(ss)
							m_nightclock = !!val;

						ss_log << " " << m_nightclock;
					}
					else if(what == "night-tint"){
						unsigned int r,g,b,a;

						ss >> std::hex >> r >> g >> b >> a;

						if(ss && (r <= 0xff) && (g <= 0xff) && (b <= 0xff) && (a <= 0xff)){
							m_night_tint.r = r;
							m_night_tint.g = g;
							m_night_tint.b = b;
							m_night_tint.a = a;
						}

						ss_log << hex << setfill('0')
							<< " " << setw(2) << (int) m_night_tint.r
							<< " " << setw(2) << (int) m_night_tint.g
							<< " " << setw(2) << (int) m_night_tint.b
							<< " " << setw(2) << (int) m_night_tint.a;
					}

					// Toggle interlace/alternating scanline 3D render mode.
					else if(what == "interlace"){
						int val;
						ss >> val;

						if(ss){
							cam->m_interlace = !!val;
						}

						ss_log << " " << cam->m_interlace;
					}

					// Toggle wireframe overlay on 3D meshes.
					else if(what == "wireframe"){
						int val;
						ss >> val;

						if(ss){
							cam->m_wireframe = !!val;
						}

						ss_log << " " << cam->m_wireframe;
					}

					// Fullscreen or windowed mode
					else if(what == "fullscreen"){
						bool val;
						ss >> val;

						if(ss){
							if(val != ctrl->fullscreen){
								ctrl->fullscreen = val;

								SDL_SetWindowFullscreen(ctrl->win, (
									ctrl->fullscreen ?
										SDL_WINDOW_FULLSCREEN_DESKTOP :
										0
								));

								SDL_Delay(500);
							}
						}

						ss_log << " " << ctrl->fullscreen;
					}

					// Size of window scale
					else if(what == "scale"){
						int scale;
						ss >> scale;

						if(ss && !ctrl->fullscreen)
							ctrl->set_render_scale(scale);

						ss_log << " " << render_scale;
					}

					else valid = false;
				} else valid = false;
			} else valid = false;
		} else valid = false;

		// Command as we understood it.
		// FIXME debug
		if(valid){
			// Send command as we understood it to the console log.
			m_console.log_cmd(ss_log.str());

			// FIXME debug
			cout << ss_log.str() << endl;
		} else if(ss && (verb == "help")){
			// Help output, possibly with command-specific help.
			// TODO

			// FIXME debug
			m_console.log("FIXME help");

			// FIXME debug
			cout << "Help!" << endl;
		} else {
			string msg = string("ERR: ") + ss_log.str() + "? Try HELP.";

			// Send error message to console log.
			m_console.log(msg);

			// FIXME debug
			cout << msg << endl;
		}

		// TODO - if !valid or if verb == help, print help statement.
	}

	void draw(int ticks){
		m_tickcounter += ticks;

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
				walk_speed *= 5;

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
			double turn_speed = M_PI * ticks / 1000.0; // 180 degrees per second
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

			int hour = ((m_tickcounter / 1000) % 48) / 2;
			ss << "tod: " << hour << "; fps: " << (fps / 60);

			text_debug->set_message(ss.str());
		}
		/* on-screen debug */

		Scene3D::draw(ticks);

		if(m_nightmode){
			SDL_Rect fsrect = (SDL_Rect){
				0, 0,
				SCREEN_WIDTH, SCREEN_HEIGHT
			};

			uint8_t intensity = m_night_tint.a;

			// Adjust perception of night over time.
			if(m_nightclock){
				int hour = ((m_tickcounter / 1000) % 48) - 24;

				if(hour < 0){
					intensity *= ((double) hour / - 24.0);
				} else {
					intensity *= ((double) hour / 24.0);
				}
			}

			SDL_SetRenderDrawColor(rend, m_night_tint.r, m_night_tint.g, m_night_tint.b, intensity);
			SDL_RenderFillRect(rend, &fsrect);
		}

		if(m_console_pending){
			ModalInputText *pConsoleScene = new ModalInputText(ctrl);

			// Disable mouse look so that the mouse cursor will be available in the dialog.
			cam->mlook(false);

			m_console_pending = false;

			pConsoleScene->set_console(&m_console);
			ctrl->scene_descend(pConsoleScene);
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
