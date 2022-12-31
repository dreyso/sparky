#include "../header/game.h"
#include "../header/screen_size.h"
#include "../header/timer.h"
#include "../header/collision.h"
#include "../header/pathfinder.h"

// Temporary, should be handled by an entity manager class
#include "../header/components/component.h"
#include "../header/components/mechanical_component.h"
#include "../header/components/behavior_component.h"
#include "../header/components/key_press_accel_component.h"
#include "../header/components/texture_component.h"
#include "../header/components/camera_component.h"
#include "../header/components/map_collision_component.h"

#include "../header/util.h"

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>


GameLoader::GameLoader()
{
	// Only one instance of Game is allowed
	++mGameObjCount;
	if (mGameObjCount > 1)
	{
		fprintf(stderr, "More than 1 instance of Game is detected, when only 1 is allowed\n");
		exit(0);
	}

	// Quit program if game loading fails
	if (init() == false)
		exit(-1);
}

GameLoader::~GameLoader()
{
	close();
}

bool GameLoader::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Create window
	mWindow.reset(SDL_CreateWindow("SDL 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS));
	if (mWindow == nullptr)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Create renderer
	mRenderer.reset(SDL_CreateRenderer(mWindow.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (mRenderer == nullptr)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Enable blendng
	if (SDL_SetRenderDrawBlendMode(mRenderer.get(), SDL_BLENDMODE_BLEND) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Enable anti-aliasing
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") != SDL_TRUE)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Set renderer color
	if (SDL_SetRenderDrawColor(mRenderer.get(), 0x05, 0xbc, 0xe3, 0xFF) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	// Initialize SDL_image
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		fprintf(stderr, "%s", IMG_GetError());
		return false;
	}

	// Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		fprintf(stderr, "%s", Mix_GetError());
		return false;
	}

	// Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		fprintf(stderr, "%s", TTF_GetError());
		return false;
	}
	
	return true;
}
void GameLoader::close()
{
	// Preferable to free window and renderer before closing SDL
	mRenderer.reset(nullptr);
	mWindow.reset(nullptr);

	// Quit SDL subsystems
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

Game::Game() : GameLoader{}, mMap{ mRenderer.get() }
{
	if (loadAssets() == false)
		exit(-1);
	
	// Initialize the timer for first frame
	mTimer.start();

	// Temp testing
	player.addComponent<MechanicalComponent>(SDL_FRect{ 2200.f, 1500.f, 30.f, 30.f });
	// player.addComponent<BehaviorComponent>(&mMap, &player, 3500.f, 500.f, 7000.f);
	player.addComponent<KeyPressAccelComponent>(&mEvent, 5000.f, 500.f, 7000.f);
	player.addComponent<CameraComponent>(mWindow.get());
	//sob.addComponent<TextureComponent>(&mPlayer.getCamera(), mRenderer.get(), "assets/images/triangle.png");
	player.addComponent<SharedTextureComponent<int>>(&player.getComponent<CameraComponent>().getCamera(), mRenderer.get(), "assets/images/triangle.png");
	player.addComponent<MapCollisionComponent>(&mMap);


	sob.addComponent<MechanicalComponent>(SDL_FRect{ 2200.f, 1700.f, 30.f, 30.f });
	sob.addComponent<BehaviorComponent>(&mMap, &player, 3500.f, 500.f, 7000.f);
	// sob.addComponent<KeyPressAccelComponent>(&mEvent, 5000.f, 500.f, 7000.f);
	// sob.addComponent<CameraComponent>(mWindow.get());
	// sob.addComponent<TextureComponent>(&mPlayer.getCamera(), mRenderer.get(), "assets/images/triangle.png");
	sob.addComponent<SharedTextureComponent<int>>(&player.getComponent<CameraComponent>().getCamera());

	bob.addComponent<MechanicalComponent>(SDL_FRect{ 100.f, 100.f, 30.f, 30.f });
	bob.addComponent<BehaviorComponent>(&mMap, &sob, 3500.f, 500.f, 7000.f);
	bob.addComponent<SharedTextureComponent<int>>(&player.getComponent<CameraComponent>().getCamera());
	//bob.addComponent<TextureComponent>(&mPlayer.getCamera(), mRenderer.get(), "assets/images/triangle.png");
}

Game::~Game()
{
	freeAssets();
}

bool Game::loadAssets()
{
	// Load fontfont
	mFont = TTF_OpenFont("assets/calibri_regular.ttf", 26);
	if (mFont == nullptr)
	{
		fprintf(stderr, "%s", TTF_GetError());
		return false;
	}

	// Load soundtrack
	mSoundtrack = Mix_LoadMUS("assets/sound/epic-pop-dance.mp3");
	if (mSoundtrack == nullptr)
	{
		fprintf(stderr,"%s", Mix_GetError());
		return false;
	}

	// Load sound effect
	mSoundEffect = Mix_LoadWAV("assets/sound/test5.wav");
	if (mSoundEffect == nullptr)
	{
		fprintf(stderr,"%s", Mix_GetError());
		return false;
	}
	return true;
}

void Game::handleEvents()
{
	if (mPaused == true)
		printf("lol");
	// Handle all events
	while (SDL_PollEvent(&mEvent) != 0 || mPaused)
	{
		// User quits
		if (mEvent.type == SDL_QUIT)
		{
			mRunning = false;
		}
		// User leaves window
		else if (mEvent.type == SDL_WINDOWEVENT)
		{
			if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				SDL_SetWindowGrab(mWindow.get(), SDL_TRUE);
				mPaused = false;
			}
			else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				mPaused = true;
				SDL_SetWindowGrab(mWindow.get(), SDL_FALSE);
			}
		}
		// User presses a key
		else if (mEvent.type == SDL_KEYDOWN)
		{
			switch (mEvent.key.keysym.sym)
			{
				// Play the music
			case SDLK_p:
			{
				// If there is no music playing
				if (Mix_PlayingMusic() == 0)
				{
					// Play the music
					Mix_PlayMusic(mSoundtrack, -1);
				}
				// If music is being played
				else
				{
					// If the music is mPaused
					if (Mix_PausedMusic() == 1)
					{
						// Resume the music
						Mix_ResumeMusic();
					}
					// If the music is playing
					else
					{
						// Pause the music
						Mix_PauseMusic();
					}
				}
				break;
			}
			// Fade the music
			case SDLK_o:
			{
				Mix_FadeOutMusic(5000);
				break;
			}
			}
		}
		else if (mEvent.type == SDL_MOUSEMOTION/* || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP*/)
		{
			// Get mouse position
			SDL_GetMouseState(&mMousePos.x, &mMousePos.y);
		}
		// Play the sound effect
		else if (mEvent.type == SDL_MOUSEBUTTONDOWN)
			Mix_PlayChannel(-1, mSoundEffect, 0);

		player.handleEvents();
	}
}

void Game::update()
{
	// Calculate time since last frame
	mDeltaTime = mTimer.getSeconds();
	
	// Restart timer
	mTimer.start();

	player.update(mDeltaTime);
	bob.update(mDeltaTime);
	sob.update(mDeltaTime);

	
}

void Game::render()
{
	// Clear screen
	SDL_RenderClear(mRenderer.get());

	// Render the player
	mMap.render(player.getComponent<CameraComponent>().getCamera());

	// Temp
	player.draw();
	bob.draw();
	sob.draw();

	// Update screen
	SDL_RenderPresent(mRenderer.get());
}

void Game::freeAssets()
{
	// Free game font
	TTF_CloseFont(mFont);
	mFont = nullptr;

	// Free game sound effect
	Mix_FreeChunk(mSoundEffect);
	mSoundEffect = nullptr;

	// Free game soundtrack
	Mix_FreeMusic(mSoundtrack);
	mSoundtrack = nullptr;
}

bool Game::isRunning(){ return mRunning; }
