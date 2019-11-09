/*
	FileLoader
	mperron (2019)

	A class which models game assets. These are instantiated via the
	automatically generated assetblob file, with base64 encoded asset
	data.
*/
#include "base64.h"

class FileLoader {
	size_t size_raw;
	const char *data_raw;
	string data;

	static map<string, FileLoader*> assets;

	SDL_RWops *rw = NULL;
	SDL_Surface *sf = NULL;
	Mix_Music *mu = NULL;
	Mix_Chunk *snd = NULL;
public:
	FileLoader(size_t size_raw, string data){
		this->size_raw = size_raw;
		this->data = data;
	}

	// Get a surface for this asset if it's an image.
	SDL_Surface *surface(){
		if(!sf)
			sf = SDL_LoadBMP_RW(rwops(), 0);

		return this->sf;
	}

	SDL_RWops *rwops(){
		if(!rw)
			rw = SDL_RWFromMem(((void*) data_raw), size_raw);

		return rw;
	}

	Mix_Music *music(){
		if(!mu)
			mu = Mix_LoadMUS_RW(rwops(), 0);

		return mu;
	}

	Mix_Chunk *sound(){
		if(!snd)
			snd = Mix_LoadWAV_RW(rwops(), 0);

		return snd;
	}

	const char *text(){
		return data_raw;
	}

	static void load(string fname, FileLoader *fl);
	static void decode_all();
	static FileLoader *get(string);
};

// A map of all the assets.
map<string, FileLoader*> FileLoader::assets;

// Called by the assetblob code to create file data.
void FileLoader::load(string fname, FileLoader *fl){
	assets[fname] = fl;
}

// Find a file by path.
FileLoader *FileLoader::get(string fname){
	return assets[fname];
}

// Turn the base64 encoded data into real data.
void FileLoader::decode_all(){
	for(auto x : assets){
		FileLoader *fl = x.second;

		fl->data_raw = base64_dec(fl->data.c_str(), strlen(fl->data.c_str()));
	}
}
