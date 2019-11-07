class Button : public Drawable, public Clickable {
	bool hover = false;
	bool down = false;

public:
	Button(
		SDL_Renderer *rend,
		SDL_Rect click_region
	) : Drawable(rend), Clickable(click_region) {
		// ...
	}

	void on_mouse_down(SDL_MouseButtonEvent event){
		if(event.button & SDL_BUTTON_LEFT)
			down = true;
	}
	void on_mouse_up(SDL_MouseButtonEvent event){
		if(event.button & SDL_BUTTON_LEFT)
			down = false;
	}
	void on_mouse_click(SDL_MouseButtonEvent event){
		action();
	}


	void on_mouse_in(SDL_MouseMotionEvent event){
		hover = true;
	}
	void on_mouse_out(SDL_MouseMotionEvent event){
		hover = false;
	}

	void draw(int ticks){
		char fill = (down ? 0x40 : 0xb0);
		char bord = ((hover && !down) ? 0xf0 : 0x70);

		// Fill
		SDL_SetRenderDrawColor(rend, fill, fill, fill, 0xFF);
		SDL_RenderFillRect(rend, &click_region);

		// Border
		SDL_SetRenderDrawColor(rend, 0x70, 0x70, bord, 0xFF);
		SDL_RenderDrawRect(rend, &click_region);
	}

	virtual void action(){}
};
