#include "SceneLoader.h"
#include "../Utils.hpp"
#include "../ECS/ECS.h"
#include "../ECS/ECS.h"
#include "../Game/Game.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/PlayerControlComponent.h"
#include "../Components/ColliderComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/ParentComponent.h"
#include "../Components/DarknessComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/PlayerInventoryComponent.h"
#include "../Components/DoorComponent.h"
#include "../Components/CollectibleComponent.h"
#include "../Components/TextComponent.h"
#include "../Components/PasswordManagerComponent.h"
#include "../Components/ScientistComponent.h"
#include "../Components/BigDoorComponent.h"
#include "../Components/ControlPanelComponent.h"
#include "../Components/TimerComponent.h"
#include "../Components/UIButtonComponent.h"
#include "glm/fwd.hpp"

auto &Registry = Registry::registry;

//////////////////////////////////////////////////
/// Helper Functions
//////////////////////////////////////////////////

inline int PseudoRandomTileOffset(int x, int y, int range) {
    unsigned h = x * 73856093u ^ y * 19349663u;
    return int(h % range);
}

// Gap bit positions
enum GapSide { TOP=0, RIGHT=1, BOTTOM=2, LEFT=3 };

// Predefined gap sets
constexpr std::bitset<4> GAPS_NONE                = std::bitset<4>(0b0000);
constexpr std::bitset<4> GAPS_4WAY                = std::bitset<4>(0b1111);
constexpr std::bitset<4> GAPS_BOTTOM_RIGHT        = std::bitset<4>( (1<<BOTTOM) | (1<<RIGHT) );
constexpr std::bitset<4> GAPS_UP                  = std::bitset<4>( 1<<TOP );
constexpr std::bitset<4> GAPS_UP_BOTTOM_RIGHT     = std::bitset<4>( (1<<TOP) | (1<<BOTTOM) | (1<<RIGHT) );
constexpr std::bitset<4> GAPS_LEFT                = std::bitset<4>( 1<<LEFT );
constexpr std::bitset<4> GAPS_LEFT_RIGHT       = std::bitset<4>((1<<LEFT) | (1<<RIGHT));
constexpr std::bitset<4> GAPS_BOTTOM_UP       = std::bitset<4>( (1<<TOP) | (1<<BOTTOM) );
constexpr std::bitset<4> GAPS_BOTTOM           = std::bitset<4>(1<<BOTTOM);
constexpr std::bitset<4> GAPS_LEFT_RIGHT_BOTTOM = std::bitset<4>((1<<LEFT) | (1<<RIGHT) | (1<<BOTTOM));

inline void CreateRoom(
    int wTiles, int hTiles,
    glm::vec2 startPos,
    const std::string& floorTile, int floorRange,
	bool haveWalls,
    const std::string& wallTile = "",  int wallRange = 0,
    std::bitset<4> gaps = std::bitset<4>(0),
    int doorWidthTiles = 3
) {
    const float S = SCALE_FACTOR_32;
    const int TW = 32, TH = 32;
    const int stepX = int(TW * S), stepY = int(TH * S);
    const int x0 = int(startPos.x), y0 = int(startPos.y);
    const int cx = wTiles / 2, cy = hTiles / 2;
    const int halfDoorX = doorWidthTiles / 2;
    const int halfDoorY = doorWidthTiles / 2;

    for (int ty = 0; ty < hTiles; ++ty) {
        for (int tx = 0; tx < wTiles; ++tx) {
            const bool onLeft   = (tx == 0);
            const bool onRight  = (tx == wTiles - 1);
            const bool onTop    = (ty == 0);
            const bool onBottom = (ty == hTiles - 1);
            bool border = onLeft || onRight || onTop || onBottom;

            if (border) {
                if (onTop    && gaps[TOP]    && std::abs(tx - cx) <= halfDoorX)    border = false;
                if (onBottom && gaps[BOTTOM] && std::abs(tx - cx) <= halfDoorX)    border = false;
                if (onLeft   && gaps[LEFT]   && std::abs(ty - cy) <= halfDoorY)    border = false;
                if (onRight  && gaps[RIGHT]  && std::abs(ty - cy) <= halfDoorY)    border = false;
            }

            const bool useWall = haveWalls ? border : false;
            const int  range   = useWall ? wallRange : floorRange;
            const auto& atlas  = useWall ? wallTile  : floorTile;

            const int x = x0 + tx * stepX;
            const int y = y0 + ty * stepY;

            Entity e = Registry->CreateEntity();
            e.AddComponent<TransformComponent>(glm::vec2(x, y), glm::vec2(S));

            int variant = (range > 0) ? PseudoRandomTileOffset(x, y, range) : 0;
            e.AddComponent<SpriteComponent>(atlas, TW, TH, useWall ? 3 : 0, false, TW * variant, 0);

            if (useWall) {
                e.AddComponent<ColliderComponent>(Box, glm::vec2(0,0), int(TW * S), int(TH * S));
                e.AddTag(Obstacle);
            }
        }
    }
}

inline glm::vec2 InTiles(float x, float y) {
	return glm::vec2(32 * SCALE_FACTOR_32 * x, 32 * SCALE_FACTOR_32 * y);
}

//////////////////////////////////////////////////
/// Member Functions
//////////////////////////////////////////////////
void SceneLoader::UnloadCurrentScene() {
	Registry->ClearEntities();
}

void SceneLoader::LoadScene(Scenes level) {
	switch (level) {
	case StartMenu: {
			SDL_Color white = {255, 255, 255};
			//---//EntityName//---//
			//Entity entity = Registry->CreateEntity();
			//entity.AddComponent<ComponentType>(args);

			//---//PlayerCast//---//
			Entity playerInteractionCast = Registry->CreateEntity();
			playerInteractionCast.AddComponent<TransformComponent>(
				glm::vec2(0),
				glm::vec2(1)
			);
			playerInteractionCast.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0),
				PLAYER_INTERACTION_WIDTH,
				PLAYER_INTERACTION_HEIGHT
			);
			playerInteractionCast.AddComponent<PlayerInventoryComponent>(
				/*key*/ false,
				/*card*/ false,
				/*bomb*/ false
			);
			playerInteractionCast.AddTag(PlayerInteractionCast);

			//---//Player//---//
			Entity player = Registry->CreateEntity();
			player.AddComponent<TransformComponent>(
				// glm::vec2(Game::windowWidth/2 - 32, Game::windowHeight/2 - 32),
				glm::vec2(0, 0),
				glm::vec2(SCALE_FACTOR_32)
			);
			player.AddComponent<SpriteComponent>("time-bot", 32, 32, 5);
			player.AddComponent<AnimationComponent>(AnimationComponent{
				{"IdleSouth", 0, 4, true, 6},
				{"WalkSouth", 1, 4, true, 6},
				{"IdleEast" , 2, 4, true, 6},
				{"WalkEast" , 3, 4, true, 6},
				{"IdleNorth", 4, 4, true, 6},
				{"WalkNorth", 5, 4, true, 6},
				{"IdleWest" , 6, 4, true, 6},
				{"WalkWest" , 7, 4, true, 6},
			});
			player.AddComponent<RigidBodyComponent>(
				glm::vec2(0,0)
			);
			player.AddComponent<PlayerControlComponent>(400.0);
			player.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			player.AddComponent<ParentComponent>(ParentComponent{
				{playerInteractionCast, glm::vec2(0)},
			});
			player.AddComponent<CameraFollowComponent>();
			player.AddTag(Player);

			//---//Card Door//---//
			Entity cardDoor = Registry->CreateEntity();
			cardDoor.AddComponent<TransformComponent>(
				glm::vec2(-384, 0),
				glm::vec2(SCALE_FACTOR_32)
			);
			cardDoor.AddComponent<SpriteComponent>(
				"misc", 32, 32, 5, false, 224, 0
			);
			cardDoor.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			cardDoor.AddComponent<DoorComponent>(CardDoor);
			cardDoor.AddTag(Obstacle);

			//---//Card//---//
			Entity card = Registry->CreateEntity();
			card.AddComponent<TransformComponent>(
				glm::vec2(256, -1088),
				glm::vec2(SCALE_FACTOR_32)
			);
			card.AddComponent<SpriteComponent>(
				"misc", 32, 32, 5, false, 32, 0
			);
			card.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			card.AddComponent<CollectibleComponent>(Card);
			card.AddTag(Collectible);
			
			Entity garbage1 = Registry->CreateEntity();
			garbage1.AddComponent<SpriteComponent>(
				"misc", 32, 32, 3, false, 0, 0
			);
			garbage1.AddComponent<TransformComponent>(
				glm::vec2(256, -1152),
				glm::vec2(SCALE_FACTOR_32)
			);
			Entity garbage2 = Registry->CreateEntity();
			garbage2.AddComponent<SpriteComponent>(
				"misc", 32, 32, 3, false, 0, 0
			);
			garbage2.AddComponent<TransformComponent>(
				glm::vec2(192, -1152),
				glm::vec2(SCALE_FACTOR_32)
			);
			Entity garbage3  = Registry->CreateEntity();
			garbage3.AddComponent<SpriteComponent>(
				"misc", 32, 32, 3, false, 0, 0
			);
			garbage3.AddComponent<TransformComponent>(
				glm::vec2(192, -1088),
				glm::vec2(SCALE_FACTOR_32)
			);

			//---//Key//---//
			Entity key = Registry->CreateEntity();
			key.AddComponent<TransformComponent>(
				glm::vec2(1344, 1344),
				glm::vec2(SCALE_FACTOR_32)
			);
			key.AddComponent<SpriteComponent>(
				"misc", 32, 32, 5, false, 64, 0
			);
			key.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			key.AddComponent<CollectibleComponent>(Key);
			key.AddTag(Collectible);

			//---//Bomb//---//
			Entity bomb = Registry->CreateEntity();
			bomb.AddComponent<TransformComponent>(
				glm::vec2(-2884, -190),
				glm::vec2(SCALE_FACTOR_32)
			);
			bomb.AddComponent<SpriteComponent>(
				"misc", 32, 32, 5, false, 96, 0
			);
			bomb.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			bomb.AddComponent<CollectibleComponent>(Bomb);
			bomb.AddTag(Collectible);

			//---//Key Door//---//
			Entity keyDoor = Registry->CreateEntity();
			keyDoor.AddComponent<TransformComponent>(
				glm::vec2(384, 0),
				glm::vec2(SCALE_FACTOR_32)
			);
			keyDoor.AddComponent<SpriteComponent>(
				"misc", 32, 32, 5, false, 192, 0
			);
			keyDoor.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			keyDoor.AddComponent<DoorComponent>(KeyDoor);
			keyDoor.AddTag(Obstacle);

			//---//Bomb Door//---//
			constexpr int yVal = 64 * 27;
			Entity bombDoor = Registry->CreateEntity();
			bombDoor.AddComponent<TransformComponent>(
				glm::vec2(0, yVal),
				glm::vec2(SCALE_FACTOR_32)
			);
			bombDoor.AddComponent<SpriteComponent>(
				"bomba", 32, 32, 5
			);
			bombDoor.AddComponent<AnimationComponent>(AnimationComponent{
				{"chill", 0, 1, true, 6},
				{"boom", 1, 21, false, 6},
				{"dead", 2, 1, false, 6}
			});
			bombDoor.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			bombDoor.AddComponent<DoorComponent>(BombDoor);
			bombDoor.AddTag(Obstacle);

			//---//Battery Room//---//
			Entity rw0 = Registry->CreateEntity();
			rw0.AddComponent<TransformComponent>(
				glm::vec2(128, -1600),
				glm::vec2(SCALE_FACTOR_32)
			);
			rw0.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);
			rw0.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32 * 3
			);
			rw0.AddTag(Obstacle);
			Entity rw1 = Registry->CreateEntity();
			rw1.AddComponent<TransformComponent>(
				glm::vec2(128, -1536),
				glm::vec2(SCALE_FACTOR_32)
			);
			rw1.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);
			Entity rw2 = Registry->CreateEntity();
			rw2.AddComponent<TransformComponent>(
				glm::vec2(128, -1472),
				glm::vec2(SCALE_FACTOR_32)
			);
			rw2.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);

			Entity lw0 = Registry->CreateEntity();
			lw0.AddComponent<TransformComponent>(
				glm::vec2(-128, -1600),
				glm::vec2(SCALE_FACTOR_32)
			);
			lw0.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);
			lw0.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32 * 3
			);
			lw0.AddTag(Obstacle);
			Entity lw1 = Registry->CreateEntity();
			lw1.AddComponent<TransformComponent>(
				glm::vec2(-128, -1536),
				glm::vec2(SCALE_FACTOR_32)
			);
			lw1.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);
			Entity lw2 = Registry->CreateEntity();
			lw2.AddComponent<TransformComponent>(
				glm::vec2(-128, -1472),
				glm::vec2(SCALE_FACTOR_32)
			);
			lw2.AddComponent<SpriteComponent>(
				"metal-wall", 32, 32, 5
			);

			//---//Password Manager//---//
			Entity passwordManager = Registry->CreateEntity();
			passwordManager.AddComponent<TransformComponent>(glm::vec2(
				(Game::windowWidth - 100),
				(Game::windowHeight - 100)
			));
			passwordManager.AddComponent<TextComponent>("Password: _ _ _", "pico-40", white, TopRight);
			passwordManager.AddComponent<PasswordManagerComponent>();
			passwordManager.AddTag(PasswordManager);

			//---//Timer//---//
			Entity timer = Registry->CreateEntity();
			timer.AddComponent<TransformComponent>(glm::vec2(
				(50),
				(50)
			));
			timer.AddComponent<TextComponent>("00:00", "pico-40", white, TopLeft);
			timer.AddComponent<TimerComponent>(75);

			//---//Scientist1//---//
			Entity sci1 = Registry->CreateEntity();
			sci1.AddComponent<TransformComponent>(
				glm::vec2(1472, 0),
				glm::vec2(SCALE_FACTOR_32)
			);
			sci1.AddComponent<SpriteComponent>(
				"scientist-1", 96, 32, 3
			);
			sci1.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			sci1.AddComponent<ScientistComponent>();
			sci1.AddTag(Scientist);

			//---//Scientist2//---//
			Entity sci2 = Registry->CreateEntity();
			sci2.AddComponent<TransformComponent>(
				glm::vec2(0, 2688),
				glm::vec2(SCALE_FACTOR_32)
			);
			sci2.AddComponent<SpriteComponent>(
				"scientist-2", 96, 32, 3
			);
			sci2.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			sci2.AddComponent<ScientistComponent>();
			sci2.AddTag(Scientist);

			//---//Scientist3//---//
			Entity sci3 = Registry->CreateEntity();
			sci3.AddComponent<TransformComponent>(
				glm::vec2(-2944, 1600),
				glm::vec2(SCALE_FACTOR_32)
			);
			sci3.AddComponent<SpriteComponent>(
				"scientist-3", 96, 32, 3
			);
			sci3.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(128,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			sci3.AddComponent<ScientistComponent>();
			sci3.AddTag(Scientist);

			//---//Big Door//---//
			Entity bigDoor = Registry->CreateEntity();
			bigDoor.AddComponent<TransformComponent>(
				glm::vec2(-64, -1472),
				glm::vec2(SCALE_FACTOR_32)
			);
			bigDoor.AddComponent<SpriteComponent>(
				"big-door", 96, 32, 3
			);
			bigDoor.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32 * 3,
				32 * SCALE_FACTOR_32
			);
			bigDoor.AddComponent<BigDoorComponent>();
			bigDoor.AddTag(Obstacle);
			Entity smallDoor = Registry->CreateEntity();
			smallDoor.AddComponent<TransformComponent>(
				glm::vec2(-64, -1472),
				glm::vec2(SCALE_FACTOR_32)
			);
			smallDoor.AddComponent<SpriteComponent>(
				"big-door", 96, 32, 2, false, 0, 32
			);

			//---//Control Panel//---//
			Entity panel = Registry->CreateEntity();
			panel.AddComponent<TransformComponent>(
				glm::vec2(-128, -1408),
				glm::vec2(SCALE_FACTOR_32)
			);
			panel.AddComponent<SpriteComponent>(
				"control-panel", 32, 32, 3
			);
			panel.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			panel.AddComponent<ControlPanelComponent>();
			panel.AddTag(Obstacle);

			//---//Darkness//---//
			Entity dark = Registry->CreateEntity();
			dark.AddComponent<TransformComponent>(
				glm::vec2(-256, 1088),
				glm::vec2(SCALE_FACTOR_32)
			);
			dark.AddComponent<SpriteComponent>(
				"darkness", 288, 288, 10
			);
			dark.AddComponent<AnimationComponent>(AnimationComponent{
				{"smoke", 0, 10, true, 6}
			});
			dark.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32 * 6,
				32 * SCALE_FACTOR_32
			);
			dark.AddComponent<DarknessComponent>();
			dark.AddTag(Obstacle);
			
			//---//Lever//---//
			Entity lever = Registry->CreateEntity();
			lever.AddComponent<TransformComponent>(
				glm::vec2(-1088, 1344),
				glm::vec2(SCALE_FACTOR_32)
			);
			lever.AddComponent<SpriteComponent>(
				"misc", 32, 32, 3, false, 128
			);
			lever.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			lever.AddTag(Lever);

			//---//Fan//---//
			Entity fan = Registry->CreateEntity();
			fan.AddComponent<TransformComponent>(
				glm::vec2(-256, 1344),
				glm::vec2(SCALE_FACTOR_32)
			);
			fan.AddComponent<SpriteComponent>(
				"fan", 32, 32, 3
			);
			fan.AddComponent<AnimationComponent>(AnimationComponent{
				{"spin", 0, 6, true, 6}
			});

			//---//Bridge//---//
			Entity bridge = Registry->CreateEntity();
			bridge.AddComponent<TransformComponent>(
				glm::vec2(-1984, -64),
				glm::vec2(SCALE_FACTOR_32)
			);
			bridge.AddComponent<SpriteComponent>(
				"bridge", 96, 96, 3
			);
			bridge.AddComponent<AnimationComponent>(AnimationComponent{
				{"good", 0, 1, false},
				{"crashed", 1, 2, false}
			});
			bridge.AddTag(Bridge);

			//---//Battery//---//
			Entity battery = Registry->CreateEntity();
			battery.AddComponent<TransformComponent>(
				glm::vec2(0, -1600),
				glm::vec2(SCALE_FACTOR_32)
			);
			battery.AddComponent<SpriteComponent>(
				"battery", 32, 32, 3
			);
			battery.AddComponent<AnimationComponent>(AnimationComponent{
				{"batarya", 0, 4, true, 6}
			}),
			battery.AddComponent<ColliderComponent>(
				Box,
				glm::vec2(0,0),
				32 * SCALE_FACTOR_32,
				32 * SCALE_FACTOR_32
			);
			battery.AddTag(Battery);

			//---//Ground//---//
			//main room
			CreateRoom(11, 11, InTiles(-5, -5), "metal-ground", 15, true, "metal-wall", 1, GAPS_4WAY);
			//up corridor
			CreateRoom(5, 10, InTiles(-2, -15), "metal-ground", 15, true, "metal-wall", 0, GAPS_BOTTOM_UP, 3);
			//down corriodr
			CreateRoom(5, 10, InTiles(-2, 6), "metal-ground", 15, true, "metal-wall", 0, GAPS_BOTTOM_UP, 3);
			//right corr
			CreateRoom(10, 5, InTiles(6, -2), "metal-ground", 15, true, "metal-wall", 0, GAPS_LEFT_RIGHT, 1);
			//left corr
			CreateRoom(10, 5, InTiles(-15, -2), "metal-ground", 15, true, "metal-wall", 0, GAPS_LEFT_RIGHT, 1);

			//r1 room
			CreateRoom(11, 11, InTiles(16, -5), "garden-ground", 4, true, "garden-wall", 3, GAPS_LEFT);
			Entity flowers = Registry->CreateEntity();
			flowers.AddComponent<TransformComponent>(
				glm::vec2(1088, -256),
				glm::vec2(SCALE_FACTOR_32)
			);
			flowers.AddComponent<SpriteComponent>(
				"decor", 288, 288, 1, false, 8 * 288, 0
			);

			//u1 room
			CreateRoom(11, 11, InTiles(-5, -26), "metal-ground", 15, true, "metal-wall", 1, GAPS_BOTTOM);

			//d1 room
			CreateRoom(11, 11, InTiles(-5, 16), "metal-ground", 15, true, "metal-wall", 1, GAPS_UP_BOTTOM_RIGHT);
			//d1 down corr
			CreateRoom(5, 10, InTiles(-2, 27), "metal-ground", 15, true, "metal-wall", 0, GAPS_BOTTOM_UP, 1);
			//d1 right corr
			CreateRoom(10, 5, InTiles(6, 19), "metal-ground", 15, true, "metal-wall", 0, GAPS_LEFT_RIGHT, 3);

			//d1r1 room
			CreateRoom(11, 11, InTiles(16, 16), "metal-ground", 15, true, "metal-wall", 1, GAPS_LEFT);

			//d2 room
			CreateRoom(11, 11, InTiles(-5, 37), "metal-ground", 15, true, "metal-wall", 1, GAPS_UP);

			//l1 room
			CreateRoom(11, 11, InTiles(-26, -5), "metal-ground", 15, true, "metal-wall", 1, GAPS_LEFT_RIGHT_BOTTOM);
			//l1 left corr
			CreateRoom(10, 5, InTiles(-36, -2), "metal-ground", 15, true, "metal-wall", 0, GAPS_LEFT_RIGHT, 3);
			//l1 down corr
			CreateRoom(5, 10, InTiles(-23, 6), "metal-ground", 15, true, "metal-wall", 0, GAPS_BOTTOM_UP, 3);
			Entity l1room = Registry->CreateEntity();
			l1room.AddComponent<TransformComponent>(
				glm::vec2(-1600, -256),
				glm::vec2(SCALE_FACTOR_32)
			);
			l1room.AddComponent<SpriteComponent>(
				"decor", 288, 288, 1, false, 2 * 288, 0
			);


			//l1d1 room
			CreateRoom(11, 11, InTiles(-26, 16), "metal-ground", 15, true, "metal-wall", 1, GAPS_UP);

			//l2 room
			CreateRoom(11, 11, InTiles(-47, -5), "metal-ground", 15, true, "metal-wall", 1, GAPS_BOTTOM_RIGHT);
			//l2 down corr
			CreateRoom(5, 10, InTiles(-44, 6), "metal-ground", 15, true, "metal-wall", 0, GAPS_BOTTOM_UP, 3);

			//l2d1 room
			CreateRoom(11, 11, InTiles(-47, 16), "metal-ground", 15, true, "metal-wall", 1, GAPS_UP);

			break;
		}
	case Gameplay:{
			SDL_Color white = {255, 255, 255};
			SDL_Color red = {255, 0, 0};
			int midX = Game::windowWidth/2.0;

			Entity deathText = Registry->CreateEntity();
			deathText.AddComponent<TransformComponent>(glm::vec2(midX, 350));
			deathText.AddComponent<TextComponent>("Batteries Not Included", "pico-60", white, TopCenter);

			// start button
			Entity restart = Registry->CreateEntity();
			restart.AddComponent<TransformComponent>(glm::vec2(midX, 500));
			restart.AddComponent<TextComponent>("Start", "pico-40", white, TopCenter);
			restart.AddComponent<UIButtonComponent>(300, 50, [this]() {
				UnloadCurrentScene();
				LoadScene(StartMenu);
			}, glm::vec2(-150, -40));
			restart.AddComponent<CameraFollowComponent>();

			// Quit button
			Entity quit = Registry->CreateEntity();
			quit.AddComponent<TransformComponent>(glm::vec2(midX, 600));
			quit.AddComponent<TextComponent>("Quit", "pico-40", white, TopCenter);
			quit.AddComponent<UIButtonComponent>(200, 50, []() {
				SDL_Event e;
				e.type = SDL_QUIT;
				SDL_PushEvent(&e);
			}, glm::vec2(-112, -40));

			Entity tut1 = Registry->CreateEntity();
			tut1.AddComponent<TransformComponent>(glm::vec2(
				(Game::windowWidth - 200),
				(Game::windowHeight - 150)
			));
			tut1.AddComponent<TextComponent>("WASD : Walk", "pico-20", white, TopCenter);
			tut1.AddComponent<PasswordManagerComponent>();
			tut1.AddTag(PasswordManager);

			Entity tut2 = Registry->CreateEntity();
			tut2.AddComponent<TransformComponent>(glm::vec2(
				(Game::windowWidth - 200),
				(Game::windowHeight - 100)
			));
			tut2.AddComponent<TextComponent>("E : Interact", "pico-20", white, TopCenter);
			tut2.AddComponent<PasswordManagerComponent>();
			tut2.AddTag(PasswordManager);
			
			Entity tui = Registry->CreateEntity();
			tui.AddComponent<TransformComponent>(glm::vec2(
				(50),
				(Game::windowHeight - 100)
			));
			tui.AddComponent<TextComponent>("Find you batteries, before you die!", "pico-20", white, TopLeft);
			tui.AddComponent<PasswordManagerComponent>();
			tui.AddTag(PasswordManager);

			break;
		}
	case DeathScreen: {
			SDL_Color white = {255, 255, 255};
			SDL_Color red = {255, 0, 0};
			int midX = Game::windowWidth/2.0;

			// "You Died" text
			Entity deathText = Registry->CreateEntity();
			deathText.AddComponent<TransformComponent>(glm::vec2(midX, 350));
			deathText.AddComponent<TextComponent>("Search simulation failed...", "pico-60", white, TopCenter);

			// Restart button
			Entity restart = Registry->CreateEntity();
			restart.AddComponent<TransformComponent>(glm::vec2(midX, 500));
			restart.AddComponent<TextComponent>("Re-calculate", "pico-40", white, TopCenter);
			restart.AddComponent<UIButtonComponent>(300, 50, [this]() {
				UnloadCurrentScene();
				LoadScene(StartMenu);   // restart game
			}, glm::vec2(-150, -40));
			restart.AddComponent<CameraFollowComponent>();

			// Quit button
			Entity quit = Registry->CreateEntity();
			quit.AddComponent<TransformComponent>(glm::vec2(midX, 600));
			quit.AddComponent<TextComponent>("Quit", "pico-40", white, TopCenter);
			quit.AddComponent<UIButtonComponent>(200, 50, []() {
				SDL_Event e;
				e.type = SDL_QUIT;
				SDL_PushEvent(&e);
			}, glm::vec2(-112, -40));


			break;
		}

	case WinScene: {
			SDL_Color white = {255, 255, 255};
			SDL_Color red = {255, 0, 0};
			int midX = Game::windowWidth/2.0;

			// "You Died" text
			Entity deathText = Registry->CreateEntity();
			deathText.AddComponent<TransformComponent>(glm::vec2(midX, 350));
			deathText.AddComponent<TextComponent>("Yes, that should work...", "pico-60", white, TopCenter);

			// Restart button
			Entity restart = Registry->CreateEntity();
			restart.AddComponent<TransformComponent>(glm::vec2(midX, 500));
			// restart.AddComponent<TextComponent>("Re-calculate", "pico-40", white, TopCenter);
			// restart.AddComponent<UIButtonComponent>(300, 50, [this]() {
			// 	UnloadCurrentScene();
			// 	LoadScene(StartMenu);   // restart game
			// }, glm::vec2(-150, -40));
			restart.AddComponent<CameraFollowComponent>();

			// Quit button
			Entity quit = Registry->CreateEntity();
			quit.AddComponent<TransformComponent>(glm::vec2(midX, 600));
			quit.AddComponent<TextComponent>("Quit", "pico-40", white, TopCenter);
			quit.AddComponent<UIButtonComponent>(200, 50, []() {
				SDL_Event e;
				e.type = SDL_QUIT;
				SDL_PushEvent(&e);
			}, glm::vec2(-112, -40));


			break;
		}
	}
}
