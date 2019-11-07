class GarageScene : public Scene {
public:
	GarageScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "garage.bmp");
	}
};
