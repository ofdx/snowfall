/*
	Clickable
	mperron(2019)

	Classes which inherit from this one can define a clickable region,
	and then implement the various event functions:

		void on_mouse_click(SDL_MouseButtonEvent event){}

		void on_mouse_down(SDL_MouseButtonEvent event){}
		void on_mouse_up(SDL_MouseButtonEvent event){}

		void on_mouse_in(SDL_MouseMotionEvent event){}
		void on_mouse_out(SDL_MouseMotionEvent event){}
*/
class Clickable {
	bool mouse_in = false;
	int buttons = 0;

	// Returns true if the mouse_in state changed.
	bool is_mouse_in(int screen_x, int screen_y){
		bool was_mouse_in = mouse_in;

		// Check if the mouse is inside the region.
		mouse_in = (
			((screen_x / render_scale) >= click_region.x) &&
			((screen_y / render_scale) >= click_region.y) &&

			((screen_x / render_scale) < (click_region.x + click_region.w)) &&
			((screen_y / render_scale) < (click_region.y + click_region.h))
		);

		return (mouse_in != was_mouse_in);
	}

protected:
	// The region which is considered clickable.
	SDL_Rect click_region = { 0, 0, 0, 0 };

public:
	Clickable(){
	}

	Clickable(SDL_Rect click_region){
		this->click_region = click_region;
	}

	void check_mouse(SDL_Event event){
		switch(event.type){
			case SDL_MOUSEMOTION:
			{
				if(is_mouse_in(event.motion.x, event.motion.y)){
					if(mouse_in)
						on_mouse_in(event.motion);
					else
						on_mouse_out(event.motion);
				}

				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				// Check if the mouse is in, because the region may have been
				// changed by set_click_region() without the mouse cursor
				// generating a new SDL_MOUSEMOTION event.
				is_mouse_in(event.button.x, event.button.y);

				if(mouse_in){
					buttons |= event.button.button;
					on_mouse_down(event.button);
				}
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				// You might be tempted to call is_mouse_in() here, just like
				// the SDL_MOUSEBUTTONDOWN handler, but I think this is
				// undesirable. The net effect would be, if an object is moving
				// the user successfully clicked on it, it could then escape
				// before they had time to release the mouse.

				buttons &= !event.button.button;
				on_mouse_up(event.button);

				if(mouse_in)
					on_mouse_click(event.button);
				break;
			}
		}
	}

	void set_click_region(SDL_Rect click_region){
		this->click_region = click_region;
	}

protected:
	virtual void on_mouse_click(SDL_MouseButtonEvent event){}

	virtual void on_mouse_down(SDL_MouseButtonEvent event){}
	virtual void on_mouse_up(SDL_MouseButtonEvent event){}

	virtual void on_mouse_in(SDL_MouseMotionEvent event){}
	virtual void on_mouse_out(SDL_MouseMotionEvent event){}
};
