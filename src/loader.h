/*
	FileLoader
	mperron (2019)

	A class which models game assets. These are instantiated via the
	automatically generated assetblob file.
*/

class FileLoader {
	size_t size_raw;
	char *data_raw;

	static map<string, FileLoader*> assets;

	SDL_RWops *rw = NULL;
	SDL_Surface *sf = NULL;
	Mix_Music *mu = NULL;
	Mix_Chunk *snd = NULL;
public:
	FileLoader(size_t size_raw, unsigned char *data){
		this->size_raw = size_raw;
		this->data_raw = (char*) data;
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
	FileLoader *fl = assets[fname];

	if(!fl)
		cerr << "File not found: " << fname << endl;

	return fl;
}
