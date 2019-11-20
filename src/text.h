/*
	PicoText
	mperron (2019)

	A class which draws text onto the screen using a bitmap font.
*/
class PicoText : public Drawable {
	SDL_Texture *font;
	SDL_Rect region;
	string message;

	// Size of the individual letters in pixels. This includes an extra pixel
	// on the right and bottom which isn't normally used, that represents the
	// space between characters and lines of text. Some characters have
	// descenders which occupy the extra bottom line.
	int c_width = 6;
	int c_height = 7;

	// Extra space between lines of text.
	int leading = 0;

	int ticks_perchar = 0;
	int ticks_passed = 0;

public:
	PicoText(SDL_Renderer *rend, SDL_Rect region, string message) : Drawable(rend) {
		this->region = region;
		this->message = message;

		// Configure optional typewriter effect...
		// TODO

		// Load the font image.
		font = textureFromBmp(rend, "fonts/6x7.bmp", true);
	}

	~PicoText(){
		SDL_DestroyTexture(font);
	}

	void set_font(string bitmap, int c_width, int c_height){
		if(font)
			SDL_DestroyTexture(font);

		font = textureFromBmp(rend, bitmap.c_str(), true);

		this->c_width = c_width;
		this->c_height = c_height;
	}

	void draw(int ticks){
		SDL_Rect src = (SDL_Rect){
			0,0,
			c_width, c_height
		};
		SDL_Rect dst = (SDL_Rect){
			region.x, region.y,
			c_width, c_height
		};
		bool wrapped = false;

		int chars_max = -1;
		if(ticks_perchar){
			unsigned int chars = ((ticks_passed + ticks) / ticks_perchar);

			// We don't want chars_max or ticks_passed to count up
			// indefinitely. Cap these values as soon as we're beyond the
			// length of the string.
			if(chars <= message.size()){
				ticks_passed += ticks;
				chars_max = chars;
			}
		}

		// FIXME This renders the text character by character on every single
		// frame regardless of whether the text has changed. If we're not doing
		// a delayed typewriter effect, and the text content or color hasn't
		// changed, we should instead read from a texture which was
		// pre-rendered. The method calls set_message and set_color could
		// trigger a redraw.
		int chars_printed = 0;
		int chars_perline = (region.w / c_width), chars_thisline = 0;
		for(char c : message){
			// Break early for the typewriter effect.
			if((chars_max >= 0) && (chars_printed++ > chars_max))
				break;

			if(wrapped){
				chars_thisline = 0;
				wrapped = false;

				// If the text just wrapped at the edge of the text region, skip a
				// following space which would be at the start of the line and look
				// bad, and a following newline which would otherwise result in a
				// double newline.
				if((c == '\n') || (c == ' '))
					continue;
			}

			// New line and carriage return.
			if(c == '\n'){
				dst.x = region.x;
				dst.y += (c_height + leading);
				chars_thisline = 0;

				continue;
			}

			if((c >= 'a') && (c <= 'z'))
				c -= 0x20;

			// The font starts at space.
			c -= ' ';

			// Unknown characters print a space.
			if(c >= 65)
				c = 0;

			src.x = (c * c_width);
			SDL_RenderCopy(rend, font, &src, &dst);
			dst.x += c_width;
			chars_thisline++;

			// Automatically wrap at the edge of the text region.
			// FIXME - this could be smarter, so it doesn't break up words.
			if(chars_thisline > chars_perline){
				dst.x = region.x;
				dst.y += (c_height + leading);
				wrapped = true;
			}
		}
	}

	void set_message(string message){
		this->message = message;
	}

	// Set the color of the text at any time.
	void set_color(char r, char g, char b){
		SDL_SetTextureColorMod(font, r, g, b);
	}

	// Set the alpha/transparency for the text at any time.
	void set_alpha(char a){
		SDL_SetTextureAlphaMod(font, a);
	}

	// Set the number of millisecond ticks to hang on each character when
	// simulating a typewriter. Set to 0 (default) to disable this effect.
	void set_ticks_perchar(int ticks){
		ticks_perchar = ticks;
	}
};
