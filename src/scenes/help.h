class HelpScene : public Scene {
public:
	HelpScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "help.bmp");
	}

	~HelpScene(){
		// FIXME debug
		{
			FileLoader *fl = FileLoader::get("desc.txt");

			if(fl)
				cout << fl->text();
		}
	}
};
