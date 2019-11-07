class Clickable {
	// The region which is considered clickable.
	SDL_Rect click_region;

	// True when the mouse is inside the region.
	bool mouse_in;

	int buttons;

public:
	Clickable(SDL_Rect click_region){
		this->click_region = click_region;
		mouse_in = false;
		buttons = 0;
	}

	void check_mouse(SDL_MouseMotionEvent event){
		bool was_mouse_in = mouse_in;

		// Check if the mouse is inside the region.
		mouse_in = (
			(event.x >= click_region.x) &&
			(event.x <= click_region.x + click_region.w) &&
			(event.y >= click_region.y) &&
			(event.y <= click_region.y + click_region.h)
		);

		if(mouse_in != was_mouse_in){
			if(mouse_in)
				on_mouse_in(event);
			else
				on_mouse_out(event);
		}

		// Call click handlers if a click event occurs.
		if(event.state & SDL_BUTTON_LMASK){
			if(!(buttons & SDL_BUTTON_LMASK)){
				buttons ^= SDL_BUTTON_LMASK;

				if(mouse_in)
					on_mouse_down(event, SDL_BUTTON_LMASK);
			}
		} else if(buttons & SDL_BUTTON_LMASK){
			buttons ^= SDL_BUTTON_LMASK;

			if(mouse_in)
				on_mouse_click(event, SDL_BUTTON_LMASK);

			// The mouse up event occurs even if the mouse came up outside the button.
			on_mouse_up(event, SDL_BUTTON_LMASK);
		}
	}

protected:
	virtual void on_mouse_click(SDL_MouseMotionEvent event, int button){}

	virtual void on_mouse_down(SDL_MouseMotionEvent event, int button){}
	virtual void on_mouse_up(SDL_MouseMotionEvent event, int button){}

	virtual void on_mouse_in(SDL_MouseMotionEvent event){}
	virtual void on_mouse_out(SDL_MouseMotionEvent event){}
};
