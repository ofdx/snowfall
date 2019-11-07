class LivingRoomScene : public Scene {
public:
	LivingRoomScene(SDL_Renderer *rend) : Scene(rend) {
		bg = textureFromBmp(rend, "living/1.bmp");
	}
};
