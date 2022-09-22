#include "../header/screen_size.h"	// defined here
#include "../header/game.h"

// Screen dimensions
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;


int main(int argc, char* args[])
{
	Game game;
	while (game.isRunning())
	{
		game.handleEvents();
		game.update();
		game.render();
	}
	
	return 0;
}
