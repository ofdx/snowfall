/*
	__game.h
	mperron (2021)

	This file serves as the main coupling between the game engine and
	the game-specific code.
*/

// Constants
#define GAME_NAME "Neige Tombe"
#define GAME_AUTHOR "Krakissi"
#define GAME_SAVEPATH "neige"
#define GAME_VERSION string("v0.01")

// Headers
#include "saveload.h"
#include "scene3d.h"
#include "wedgeterrain.h"

// Scenes
#include "scenes/intro.h"
#include "scenes/test3d.h"
#include "scenes/input_text.h"

// Register pointers to construct all of the scenes used in the game by a string
// name. There must be at least an "intro" scene which is loaded at launch.
void registerScenes(Scene::Controller *ctrl){

	// Create pointers to scene constructors by name.
	Scene::reg("intro", scene_create<IntroSplashScene>);
	Scene::reg("test3d", scene_create<TestScene3D>);

	Scene::reg("modal_input_text", scene_create<ModalInputText>);
}
