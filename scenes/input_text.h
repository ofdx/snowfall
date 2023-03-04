/*
	Modal text-input dialog screen
	mperron (2022)
*/

class ModalInputText : public Scene, public Typable {
	SDL_Texture *m_img_overlay, *m_img_mask;
	SDL_Rect m_rect_overlay;

	DebugConsole *m_pConsole;

	string m_buffer, m_buffer_base;

	bool m_sendReady, m_cancelReady;

	PicoText m_textentry;
	TextBox m_log;

	unsigned int log_index = 0;

public:
	ModalInputText(Scene::Controller *ctrl) :
		Scene(ctrl),
		Typable(true),
		m_pConsole(nullptr),
		m_sendReady(false),
		m_cancelReady(false),

		m_textentry(rend, (SDL_Rect){
			0, SCREEN_HEIGHT / 2,
			SCREEN_WIDTH, 14
		}, "> "),
		m_log(rend, (SDL_Rect){
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT / 2
		}, "")
	{
		// Scene-darkening mask.
		{
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0x7f);

			for(int x = 0, y = 0; y < SCREEN_HEIGHT; x++){
				if((x % 2) == (y % 2))
					SDL_RenderDrawPoint(rend, x, y);

				if(x == (SCREEN_WIDTH - 1)){
					x = -1;
					y++;
				}
			}
		}

		// Screenshot existing scene for a background.
		m_img_overlay = ctrl->screencap(m_rect_overlay);

		m_textentry.draw_cursor = true;
		drawables.push_back(&m_textentry);

		drawables.push_back(&m_log);
		clickables.push_back(&m_log);

		// Receive keyboard typing events.
		typables.push_back(this);
	}

	~ModalInputText(){
		if(m_img_overlay)
			SDL_DestroyTexture(m_img_overlay);
	}

	void set_history(string msg){
		m_log.set_message(msg);
		m_log.set_scroll(m_log.get_line_count() - 15);
	}

	void set_console(DebugConsole *pConsole){
		m_pConsole = pConsole;

		set_history(pConsole->log());
	}

	virtual void keydown(SDL_KeyboardEvent event) override {
		bool shift = (event.keysym.mod & KMOD_SHIFT);
		int sym = event.keysym.sym;

		switch(sym){
		case SDLK_UP:
			if(!log_index)
				m_buffer_base = m_buffer;

			m_pConsole->cmd_at(++log_index, m_buffer);
			break;
		case SDLK_DOWN:
			if(log_index > 1){
				m_pConsole->cmd_at(--log_index, m_buffer);
			} else {
				log_index = 0;
				m_buffer = m_buffer_base;
			}
			break;

		case SDLK_ESCAPE:
		case SDLK_BACKQUOTE:
			m_cancelReady = true;
			break;

		case SDLK_RETURN:
			m_sendReady = true;
			break;

		case SDLK_BACKSPACE:
			if(m_buffer.size()){
				m_buffer = m_buffer.substr(0, m_buffer.size() - 1);
			}
			break;

		case SDLK_SPACE:
			m_buffer += ' ';
			break;

		default:
			if((sym >= SDLK_a) && (sym <= SDLK_z)){
				/*if(shift){
					// Uppercase letters
					m_buffer += (char)((sym - SDLK_a) + 'A');
				} else {
					// Lowercase letters
					m_buffer += (char)((sym - SDLK_a) + 'a');
				}*/

				// The font only has one letter case, so just keep everything lower for convenience's sake.
				m_buffer += (char)((sym - SDLK_a) + 'a');
			} else if((sym >= SDLK_0) && (sym <= SDLK_9)){
				if(shift){
					// Number row symbols
					char c = ' ';

					switch(sym){
						case SDLK_1: c = '!'; break;
						case SDLK_2: c = '@'; break;
						case SDLK_3: c = '#'; break;
						case SDLK_4: c = '$'; break;
						case SDLK_5: c = '%'; break;
						case SDLK_6: c = '^'; break;
						case SDLK_7: c = '&'; break;
						case SDLK_8: c = '*'; break;
						case SDLK_9: c = '('; break;
						case SDLK_0: c = ')'; break;
					}

					m_buffer += c;
				} else {
					// Numbers
					m_buffer += (char)((sym - SDLK_0) + '0');
				}
			} else {
				char c = ' ';

				if(shift){
					switch(sym){
						case SDLK_SLASH:        c = '?'; break;
						case SDLK_SEMICOLON:    c = ':'; break;
						case SDLK_QUOTE:        c = '"'; break;
						case SDLK_MINUS:        c = '_'; break;
						case SDLK_EQUALS:       c = '+'; break;

						// Same as non-shift, for reasons noted:
						case SDLK_LEFTBRACKET:  c = '['; break;  // not in the font
						case SDLK_RIGHTBRACKET: c = ']'; break;  // not in the font
						case SDLK_BACKSLASH:    c = '\\'; break; // not in the font
						case SDLK_COMMA:        c = ','; break;  // in the font, but identical to (
						case SDLK_PERIOD:       c = '.'; break;  // in the font, but identical to )
					}
				} else {
					switch(sym){
						case SDLK_SLASH:        c = '/'; break;
						case SDLK_SEMICOLON:    c = ';'; break;
						case SDLK_QUOTE:        c = '\''; break;
						case SDLK_MINUS:        c = '-'; break;
						case SDLK_EQUALS:       c = '='; break;

						case SDLK_LEFTBRACKET:  c = '['; break;
						case SDLK_RIGHTBRACKET: c = ']'; break;
						case SDLK_BACKSLASH:    c = '\\'; break;
						case SDLK_COMMA:        c = ','; break;
						case SDLK_PERIOD:       c = '.'; break;
					}
				}

				if(c != ' ')
					m_buffer += c;
			}
		}

		m_textentry.set_message(string("> ") + m_buffer);
	}

	void draw(int ticks){
		if(m_img_overlay)
			SDL_RenderCopy(rend, m_img_overlay, &m_rect_overlay, NULL);

		// Draw text input background
		{
			SDL_Rect textbg = (SDL_Rect){
				0, SCREEN_HEIGHT / 2,
				SCREEN_WIDTH, 14
			};
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xe0);
			SDL_RenderFillRect(rend, &textbg);

			textbg = (SDL_Rect){
				0, 0,
				SCREEN_WIDTH, SCREEN_HEIGHT / 2
			};
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xa0);
			SDL_RenderFillRect(rend, &textbg);
		}

		Scene::draw(ticks);

		// Close immediately without sending a command.
		if(m_cancelReady){
			ctrl->scene_ascend();
			return;
		}

		// Send command to parent.
		if(m_sendReady)
			ctrl->scene_ascend(m_buffer);

	}
};
