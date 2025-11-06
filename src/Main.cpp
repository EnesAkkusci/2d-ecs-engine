#include "./Game/Game.h"
//TODO:
//Consider switching input handling to
//SDL_GetKeyboardState() and
//SDL_GetMouseState()

int main(int argc, char* argv[]) {
	Game game;

	game.Initalize();
	game.Run();
	game.Destroy();

	return 0;
}
