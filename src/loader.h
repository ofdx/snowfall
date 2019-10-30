#include "base64.h"

class FileLoader {
	size_t size_raw;
	const char *data_raw;
	string data;

	static map<string, FileLoader*> assets;

	SDL_RWops *rw;
	SDL_Surface *sf;
public:
	FileLoader(size_t size_raw, string data){
		this->size_raw = size_raw;
		this->data = data;

		this->sf = NULL;
	}

	// Get a surface for this asset if it's an image.
	SDL_Surface *surface(){
		if(!sf)
			sf = IMG_Load_RW(rwops(), 0);

		return this->sf;
	}

	SDL_RWops *rwops(){
		if(!rw)
			rw = SDL_RWFromMem(((void*) data_raw), size_raw);

		return rw;
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
