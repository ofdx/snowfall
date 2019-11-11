/*
	Button
	mperron (2019)

	A mouse-clickable button, which can perform an action.
*/
class Button : public Drawable, public Clickable {
	bool hover = false;
	bool down = false;

	PicoText *label;

	void label_create(string text){
		label = new PicoText(rend, (SDL_Rect){
			click_region.x + 3, click_region.y + (click_region.h / 2) - 3,
			click_region.w - 6, click_region.h - 7
		}, text);
		label->set_color(0x70, 0x70, 0x70);
	}

public:
	// Create a button which is sized by pixels (with click_region).
	Button(
		SDL_Renderer *rend,
		SDL_Rect click_region,
		string text
	) : Drawable(rend), Clickable(click_region) {
		label_create(text);
	}

	// Create a button which is sized by characters.
	Button(
		SDL_Renderer *rend,
		int x, int y,
		int rows, int cols,
		string text
	) : Drawable(rend), Clickable((SDL_Rect){
		x, y,
		((cols * 6) + 5), ((rows * 7) + 8)
	}) {
		label_create(text);
	}

	~Button(){
		delete label;
	}

	virtual void on_mouse_down(SDL_MouseButtonEvent event){
		if(event.button & SDL_BUTTON_LEFT)
			down = true;
	}
	virtual void on_mouse_up(SDL_MouseButtonEvent event){
		if(event.button & SDL_BUTTON_LEFT)
			down = false;
	}
	virtual void on_mouse_click(SDL_MouseButtonEvent event){
		action();
	}


	virtual void on_mouse_in(SDL_MouseMotionEvent event){
		hover = true;
		label->set_color(0x70, 0x70, 0xf0);
	}
	virtual void on_mouse_out(SDL_MouseMotionEvent event){
		hover = false;
		label->set_color(0x70, 0x70, 0x70);
	}

	void draw(int ticks){
		char fill = (down ? 0x40 : 0xb0);
		char bord = (hover ? 0xf0 : 0x70);

		// Fill
		SDL_SetRenderDrawColor(rend, fill, fill, fill, 0xFF);
		SDL_RenderFillRect(rend, &click_region);

		// Border
		SDL_SetRenderDrawColor(rend, 0x70, 0x70, bord, 0xFF);
		SDL_RenderDrawRect(rend, &click_region);

		// Text
		label->draw(ticks);
	}

	virtual void action(){}
};
