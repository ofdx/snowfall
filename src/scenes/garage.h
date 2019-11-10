class GarageScene : public Scene {
public:
	GarageScene(Scene::Controller *ctrl) : Scene(ctrl) {
		bg = textureFromBmp(rend, "garage.bmp");
	}
};
