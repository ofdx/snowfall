class HelpScene : public Scene {
public:
	HelpScene(Scene::Controller *ctrl) : Scene(ctrl) {
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
