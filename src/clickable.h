class Clickable {
	bool mouse_in;
	int buttons;

protected:
	// The region which is considered clickable.
	SDL_Rect click_region;

public:
	Clickable(SDL_Rect click_region){
		this->click_region = click_region;
		mouse_in = false;
		buttons = 0;
	}

	void check_mouse(SDL_Event event){
		switch(event.type){
			case SDL_MOUSEMOTION:
			{
				bool was_mouse_in = mouse_in;

				// Check if the mouse is inside the region.
				mouse_in = (
					((event.motion.x / render_scale) >= click_region.x) &&
					((event.motion.y / render_scale) >= click_region.y) &&

					((event.motion.x / render_scale) < (click_region.x + click_region.w)) &&
					((event.motion.y / render_scale) < (click_region.y + click_region.h))
				);

				if(mouse_in != was_mouse_in){
					if(mouse_in)
						on_mouse_in(event.motion);
					else
						on_mouse_out(event.motion);
				}

				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				if(mouse_in){
					buttons |= event.button.button;
					on_mouse_down(event.button);
				}
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				buttons &= !event.button.button;
				on_mouse_up(event.button);

				if(mouse_in)
					on_mouse_click(event.button);
				break;
			}
		}
	}

protected:
	virtual void on_mouse_click(SDL_MouseButtonEvent event){}

	virtual void on_mouse_down(SDL_MouseButtonEvent event){}
	virtual void on_mouse_up(SDL_MouseButtonEvent event){}

	virtual void on_mouse_in(SDL_MouseMotionEvent event){}
	virtual void on_mouse_out(SDL_MouseMotionEvent event){}
};
