#include <SDL.h>
#include "SDL_events.h"
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include "../Utils.hpp"
#include "./Game.h"
#include "../Logger/Logger.h"
#include "../SceneLoader/SceneLoader.h"

#include "../Systems/RenderSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../Systems/AnimationSystem.hpp"
#include "../Systems/CollisionSystem.hpp"
#include "../Systems/DamageSystem.hpp" //Update free, subscriber
#include "../Systems/CollisionDebugSystem.hpp"
#include "../Systems/CameraFollowSystem.hpp"
#include "../Systems/TextRenderSystem.hpp"
#include "../Systems/ImGuiRenderSystem.hpp"
#include "../Systems/UIButtonSystem.hpp"
#include "../Systems/UIDebugSystem.hpp"
#include "../Systems/LifetimeSystem.hpp"
#include "../Systems/SoundEffectSystem.hpp"
#include "../Systems/PlayerControlSystem.hpp"
#include "../Systems/ObstacleCollisionSystem.hpp"
#include "../Systems/PlayerAnimationSystem.hpp"
#include "../Systems/ParentSystem.hpp"
#include "../Systems/PlayerInteractionSystem.hpp"
#include "../Systems/DoorSystem.hpp"
#include "../Systems/CollectibleSystem.hpp"
#include "../Systems/PasswordManagerSystem.hpp"
#include "../Systems/ScientistSystem.hpp"
#include "../Systems/ControlPanelSystem.hpp"
#include "../Systems/LeverSystem.hpp"
#include "../Systems/TimerSystem.hpp"
#include "../Systems/BridgeCrashSystem.hpp"
#include "../Systems/BatterySystem.hpp"
#include "SDL_events.h"

int Game::windowHeight;
int Game::windowWidth;

Game::Game() {
	Logger::Confirm("Game constructed.");
}

Game::~Game() {
	Logger::Confirm("Game destructed.");
}

void Game::Initalize() {
	//Resolution awareness
	// SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

	if(SDL_Init(SDL_INIT_EVERYTHING)) {
		Logger::Err("Error initializing SDL.");
	} 

	if(TTF_Init() != 0) {
		Logger::Err("Error initializing TTF.");
		return;
	}

	// int flags = MIX_INIT_MP3;
	// if(!Mix_Init(flags)) {
	// 	Logger::Err("Error initializing MIXER with: " + std::string(Mix_GetError()));
	// 	return;
	// }
	
	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
		Logger::Err("Error initializing MIXER with: " + std::string(Mix_GetError()));
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	printf("SDL sees desktop as %dx%d\n", displayMode.w, displayMode.h);

	windowWidth = 1920;
	// windowWidth = 960;
	//windowWidth = displayMode.w;

	windowHeight = 1080;
	// windowHeight = 540;
	//windowHeight = displayMode.h;

	window = SDL_CreateWindow(
		"Game",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		// SDL_WINDOW_FULLSCREEN
		SDL_WINDOW_BORDERLESS
		// SDL_WINDOW_RESIZABLE
		// 0
	);
	if(!window) {
		Logger::Err("Error creating the SDL_Window.");
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(!renderer) {
		Logger::Err("Error creating the SDL_Renderer.");
		return;
	}

	//Setting up ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontGlobalScale = 2.0f;
	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);
	

	//Initalizing the camera
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;

	isGameRunning = true;
}

void Game::Run() {
	SetUp();
	while(isGameRunning) {
		ProcessInput();
		Update();
		Render();
	}
}

void Game::SetUp() {
	//Adding textures
	//asssetManager.AddTexture(renderer, "asset-name", "asset.png");
	assetManager.AddTexture(renderer, "metal-ground", "inFloor.png");
	assetManager.AddTexture(renderer, "time-bot", "TimeBotSpriteSheet4-4-4.png");
	assetManager.AddTexture(renderer, "metal-wall", "sujam6wall.png");
	assetManager.AddTexture(renderer, "garden-ground", "outFloor.png");
	assetManager.AddTexture(renderer, "garden-wall", "outWalls.png");
	assetManager.AddTexture(renderer, "misc", "miscStuff3.png");
	assetManager.AddTexture(renderer, "scientist-1", "deadsci1.png");
	assetManager.AddTexture(renderer, "scientist-2", "deadsci2.png");
	assetManager.AddTexture(renderer, "scientist-3", "deadsci3.png");
	assetManager.AddTexture(renderer, "big-door", "bigDoor.png");
	assetManager.AddTexture(renderer, "control-panel", "computer.png");
	assetManager.AddTexture(renderer, "darkness", "smokedroom.png");
	assetManager.AddTexture(renderer, "bridge", "KOPRU.png");
	assetManager.AddTexture(renderer, "battery", "batarya.png");
	assetManager.AddTexture(renderer, "fan", "BATTALFAN.png");
	assetManager.AddTexture(renderer, "decor", "roominsides.png");
	assetManager.AddTexture(renderer, "bomba", "rockboomin.png");

	//Adding fonts
	//assetManager.AddFont("font-name", "font.ttf", font-size);
	assetManager.AddFont("pico-20", "pico8.ttf", 20);
	assetManager.AddFont("pico-40", "pico8.ttf", 40);
	assetManager.AddFont("pico-60", "pico8.ttf", 60);

	//Adding sound effects
	//assetManager.AddSFX("sfx-name","sfx.wav");
	assetManager.AddSFX("click-sound", "click.wav");
	assetManager.AddSFX("bridge-crash", "metal-crash.wav");
	assetManager.AddSFX("password-sfx", "pickupCoin.wav");
	assetManager.AddSFX("explosion", "explosion.wav");

	registry.AddSystem<RenderSystem>();
	registry.AddSystem<MovementSystem>();
	registry.AddSystem<AnimationSystem>();
	registry.AddSystem<DamageSystem>();
	registry.AddSystem<CollisionSystem>();
	registry.AddSystem<CollisionDebugSystem>();
	registry.AddSystem<CameraFollowSystem>();
	registry.AddSystem<TextRenderSystem>();
	registry.AddSystem<ImGuiRenderSystem>();
	registry.AddSystem<UIButtonSystem>();
	registry.AddSystem<UIDebugSystem>();
	registry.AddSystem<LifetimeSystem>();
	registry.AddSystem<SoundEffectSystem>();
	registry.AddSystem<PlayerControlSystem>();
	registry.AddSystem<ObstacleCollisionSystem>();
	registry.AddSystem<PlayerAnimationSystem>();
	registry.AddSystem<ParentSystem>();
	registry.AddSystem<PlayerInteractionSystem>();
	registry.AddSystem<DoorSystem>();
	registry.AddSystem<CollectibleSystem>();
	registry.AddSystem<PassworManagerSystem>();
	registry.AddSystem<ScientistSystem>();
	registry.AddSystem<ControlPanelSystem>();
	registry.AddSystem<LeverSystem>();
	registry.AddSystem<TimerSystem>();
	registry.AddSystem<BridgeCrashSystem>();
	registry.AddSystem<BatterySystem>();

	sceneLoader.LoadScene(Scenes::Gameplay);

	//Systems that subscribe to events do so here
	registry.GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
	registry.GetSystem<SoundEffectSystem>().SubscribeToEvents(eventBus, &assetManager);
	registry.GetSystem<ObstacleCollisionSystem>().SubscribeToEvents(eventBus);
	registry.GetSystem<DoorSystem>().SubscribeToEvents(eventBus, &inputManager);
	registry.GetSystem<CollectibleSystem>().SubscribeToEvents(eventBus, &inputManager);
	registry.GetSystem<ScientistSystem>().SubscribeToEvents(eventBus, &inputManager, &registry);
	registry.GetSystem<ControlPanelSystem>().SubscribeToEvents(eventBus, &inputManager, &registry);
	registry.GetSystem<LeverSystem>().SubscribeToEvents(eventBus, &inputManager, &registry);
	registry.GetSystem<BridgeCrashSystem>().SubscribeToEvents(eventBus, &registry);
	registry.GetSystem<BatterySystem>().SubscribeToEvents(eventBus, &sceneLoader);
}

void Game::ProcessInput() {
	SDL_Event sdlEvent;
	inputManager.BeginFrame();
	while (SDL_PollEvent(&sdlEvent)) {
		ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

		//SDL Core inputs
		switch (sdlEvent.type) {
		case SDL_QUIT:
			isGameRunning = false;
			break;
		case SDL_KEYDOWN:
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
				isGameRunning = false;
				// registry.ClearEntities();
			}
			// if (sdlEvent.key.keysym.sym == SDLK_F1) inDebugMode = !inDebugMode;
			inputManager.KeyPressed(sdlEvent.key.keysym.scancode);
			break;
		case SDL_KEYUP:
			inputManager.KeyReleased(sdlEvent.key.keysym.scancode);
			break;
		case SDL_MOUSEBUTTONDOWN:
			inputManager.MousePressed(sdlEvent.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			inputManager.MouseReleased(sdlEvent.button.button);
			break;
		case SDL_MOUSEMOTION:
			inputManager.SetMousePosition(sdlEvent.motion.x, sdlEvent.motion.y);
		}
	}
	//System dependent on input get updated here
	registry.GetSystem<UIButtonSystem>().Update(eventBus, camera, inputManager);
	registry.GetSystem<PlayerControlSystem>().Update(inputManager);
}

void Game::Update() {
	//Limiting the FPS
	int timeToWait = MIN_MS_PER_FRAME - (SDL_GetTicks64() - msPassedUntilLastFrame);
	if (timeToWait > 0 && timeToWait <= MIN_MS_PER_FRAME) {
		SDL_Delay(timeToWait);
	}

	registry.Update();

	registry.GetSystem<MovementSystem>().Update(deltaTime);
	registry.GetSystem<AnimationSystem>().Update(deltaTime, eventBus);
	registry.GetSystem<CollisionSystem>().Update(eventBus);
	registry.GetSystem<CameraFollowSystem>().Update(camera);
	// registry.GetSystem<LifetimeSystem>().Update(deltaTime);
	registry.GetSystem<PlayerAnimationSystem>().Update();
	registry.GetSystem<ParentSystem>().Update();
	registry.GetSystem<PlayerInteractionSystem>().Update();
	registry.GetSystem<PassworManagerSystem>().Update();
	registry.GetSystem<TimerSystem>().Update(deltaTime, &sceneLoader, &eventBus);

	//Time passed between last and this frame. (Converted from ms to seconds)
	deltaTime = (SDL_GetTicks64() - msPassedUntilLastFrame) / 1000.0f;
	msPassedUntilLastFrame = SDL_GetTicks64();
}

void Game::Render() {
	// SDL_Color background = {135, 206, 235, SDL_ALPHA_OPAQUE};
	// SDL_Color background = {0, 0, 0, SDL_ALPHA_OPAQUE};
	SDL_Color background = {62, 49, 43, SDL_ALPHA_OPAQUE};
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b , background.a);
	SDL_RenderClear(renderer);

	//Systems dependent on rendering get updated here
	registry.GetSystem<RenderSystem>().Update(renderer, assetManager, camera);
	registry.GetSystem<TextRenderSystem>().Update(renderer, assetManager, camera);

	if(inDebugMode) {
		registry.GetSystem<CollisionDebugSystem>().Update(renderer, camera);
		registry.GetSystem<ImGuiRenderSystem>().Update(renderer, camera, registry, deltaTime);
		registry.GetSystem<UIDebugSystem>().Update(renderer, camera);
	}

	SDL_RenderPresent(renderer);
}

void Game::Destroy() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
