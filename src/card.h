/*
	PlayingCard
	mperron (2019)

	A playing card, with a face and back.
*/

#define PLAYINGCARD_WIDTH 30
#define PLAYINGCARD_HEIGHT 40
class PlayingCard : public Drawable, public Clickable {
public:
	enum Suit { A, B, C, D };

private:
	SDL_Texture *face, *back;
	SDL_Rect draw_region = { 0, 0, PLAYINGCARD_WIDTH, PLAYINGCARD_HEIGHT };
	enum Suit suit;
	int value;

	bool face_up = false;
	bool flipping = false;
	bool flipping_halfway = false;
	float flipping_progress = 0.0f;
	int flipping_offset = 0;

	double saturation = 1.0;
	SDL_Color color_last = { 0, 0, 0 };

	void update_color(){
		// Color card based on suit.
		{
			static const unsigned char c_min = 0x70;
			SDL_Color color = { c_min, c_min, c_min };

			unsigned char c = (0xff - ((0xff - c_min) * (1.0 - saturation)));

			switch(suit){
				case A:
					color.r = c;
					break;
				case B:
					color.r = color.b = c;
					break;
				case C:
					color.g = c;
					break;
				case D:
					color.g = color.r = c;
					break;
			}

			if(color != color_last){
				SDL_SetTextureColorMod(face, color.r, color.g, color.b);
				color_last = color;
			}
		}
	}

public:
	int offset_y = 0;
	int time_deal = 0;
	float time_progress = 0.0f;
	bool has_chirped = false;

	void set_saturation(double satch){
		this->saturation = satch;
		update_color();
	}

	void set_suit(enum Suit suit){
		this->suit = suit;
		update_color();
	}

	void set_pos(int x, int y){
		draw_region.x = x;
		draw_region.y = y;

		set_click_region(draw_region);
	}

	PlayingCard(SDL_Renderer *rend, int x, int y, enum Suit suit, int value, bool face_up) : Drawable(rend), Clickable() {
		this->face_up = face_up;
		this->value = value;

		string face_path = "cards/" + to_string(value) + ".bmp";

		face = textureFromBmp(rend, face_path.c_str(), true);
		back = textureFromBmp(rend, "cards/back.bmp", true);
		set_suit(suit);
		set_pos(x, y);
	}

	~PlayingCard(){
		if(face)
			SDL_DestroyTexture(face);
	}

	int get_value() const {
		return value;
	}

	void draw(int ticks){
		if(flipping){
			int w = slide_quad(-PLAYINGCARD_WIDTH, PLAYINGCARD_WIDTH, 200, ticks, flipping_progress);
			if(w < 0)
				w = -w;
			if(w < 2)
				w = 2;
			flipping_offset = (PLAYINGCARD_WIDTH - w) / 2;

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

	void set_face(bool up, bool immediately = true){
		if(immediately){
			flipping = false;
			face_up = up;
		} else {
			if(face_up != up)
				card_flip();
		}
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
