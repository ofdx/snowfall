/*
	Various utility methods, just so they don't have to live in main.cc.
*/


SDL_Texture *textureFromBmp(SDL_Renderer *rend, const char *fn, bool trans){
	FileLoader *fl = FileLoader::get(fn);
	if(!fl)
		return NULL;

	SDL_Surface *sf = fl->surface();
	if(sf && trans)
		SDL_SetColorKey(sf, SDL_TRUE, SDL_MapRGB(sf->format, 0xff, 0x00, 0xff));

	SDL_Texture *tx = SDL_CreateTextureFromSurface(rend, sf);

	return tx;
}
SDL_Texture *textureFromBmp(SDL_Renderer *rend, const char *fn){
	return textureFromBmp(rend, fn, false);
}

void rectSum(SDL_Rect &holder, SDL_Rect a, SDL_Rect b){
	holder.x = (a.x + b.x);
	holder.y = (a.y + b.y);
	holder.w = (a.w + b.w);
	holder.h = (a.h + b.h);
}
void rectFloatOffset(SDL_Rect &holder, SDL_Rect rect, double x, double y){
	holder.x = (int)(rect.x + x);
	holder.y = (int)(rect.y + y);
	holder.w = rect.w;
	holder.h = rect.h;
}


