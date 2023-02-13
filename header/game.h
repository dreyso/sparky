#pragma once
#include "../header/timer.h"
#include "../header/map_texture.h"
#include "../header/collision_map.h"
#include "../header/search_graph.h"
#include "../header/util.h"      // unique pointer
#include "../header/entity.h"

#include <SDL_mixer.h>
#include <SDL_ttf.h>


// Intilizes SDL
class GameLoader
{
public:
    ~GameLoader();

    bool init();
    void close();
protected:
    GameLoader();

    // Game primary window
    WindowPtr mWindow;
    RendererPtr mRenderer;

private:
    // Tracks how many instances of this class exist
    static inline int mGameObjCount = 0;
};

class Game : public GameLoader
{
public:
    Game();
    ~Game();

    bool loadAssets();
    void handleEvents();
    void update();
    void render();
    void freeAssets();

    bool isRunning();

private:
    bool mRunning = true;
    bool mPaused = false;
    float mDeltaTime = 0.f;

    // Primary font
    TTF_Font* mFont = nullptr;

    // Game's soundtrack
    Mix_Music* mSoundtrack = nullptr;

    // Game's sound effect
    Mix_Chunk* mSoundEffect = nullptr;

    // Event handler
    SDL_Event mEvent;

    // Keeps tack of mouse postion
    SDL_Point mMousePos;

    // Used to time frames
    Timer mTimer;

    // Holds the game map texture
    MapTexture mMapTexture;

    // Holds the game map
    CollisionMap mCollisionMap;

    // Holds the pathfinding data
    SearchGraph mSearchGraph;

    Entity player;
    Entity bob;
    Entity sob;
};

