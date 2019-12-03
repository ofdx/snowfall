/*
	Card
	mperron (2019)

	A playing card, with a face and back.
*/
class PlayingCard : public Drawable, public Clickable {
public:
	enum Suit { A, B, C, D };

private:
	SDL_Texture *face, *back;
	SDL_Rect draw_region = { 0, 0, 30, 40 };
	enum Suit suit;
	int value;

	bool face_up = false;
	bool flipping = false;
	bool flipping_halfway = false;
	float flipping_progress = 0.0f;
	int flipping_offset = 0;

public:
	void set_suit(enum Suit suit){
		this->suit = suit;

		// Color card based on suit.
		{
			SDL_Color color = { 0x77, 0x77, 0x77 };

			switch(suit){
				case A:
					color.r = 0xff;
					break;
				case B:
					color.b = 0xff;
					break;
				case C:
					color.g = 0xff;
					break;
				case D:
					color.r = color.g = 0xff;
					break;
			}

			SDL_SetTextureColorMod(face, color.r, color.g, color.b);
		}
	}

	void set_pos(int x, int y){
		draw_region.x = x;
		draw_region.y = y;

		set_click_region(draw_region);
	}

	PlayingCard(SDL_Renderer *rend, int x, int y, enum Suit suit, int value) : Drawable(rend), Clickable() {
		this->value = value;

		// FIXME - get the correct card image for this->value.
		face = textureFromBmp(rend, "cards/i_k.bmp", true);

		back = textureFromBmp(rend, "cards/back.bmp", true);
		set_suit(suit);
		set_pos(x, y);
	}

	~PlayingCard(){
		if(face)
			SDL_DestroyTexture(face);
	}

	void draw(int ticks){
		if(flipping){
			int w = slide_quad(-30, 30, 200, ticks, flipping_progress);
			if(w < 0)
				w = -w;
			if(w < 2)
				w = 2;
			flipping_offset = (30 - w) / 2;

			draw_region.w = w;
			set_click_region(draw_region);

			if(!flipping_halfway && (flipping_progress >= 16.0f)){
				// Visually flip the card.
				flipping_halfway = true;
			}
			if(flipping_progress >= 32.0f){
				// End the flip.
				flipping_progress = 0.0f;
				flipping = false;
			}
		}

		SDL_Rect draw_offset = {
			draw_region.x + flipping_offset, draw_region.y,
			draw_region.w, draw_region.h
		};

		SDL_Texture *face_to_show = (face_up ? face : back);
		if(flipping && !flipping_halfway)
			face_to_show = (face_up ? back: face);

		SDL_RenderCopy(rend, face_to_show, NULL, &draw_offset);
	}

	bool card_flip(){
		// This is the actual state of the card, for logic evaluations. This changes immediately.
		face_up = !face_up;

		// Initiate the graphical flip process.
		flipping = true;
		flipping_halfway = false;
		flipping_progress = 0.0f;

		return face_up;
	}

	void on_mouse_click(SDL_MouseButtonEvent event){
		switch(event.button){
			case SDL_BUTTON_LEFT:
				// Cycle through the suits for fun.
				switch(suit){
					case A:
						set_suit(B);
						break;
					case B:
						set_suit(C);
						break;
					case C:
						set_suit(D);
						break;
					case D:
						set_suit(A);
				}
				break;
			case SDL_BUTTON_RIGHT:
				// Flip the card.
				if(!flipping)
					card_flip();
				break;
		}
	}
};
