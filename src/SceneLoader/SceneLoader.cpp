#include "SceneLoader.h"
#include "../ECS/ECS.h"
#include "../ECS/ECS.h"

auto &Registry = Registry::registry;

//////////////////////////////////////////////////
/// Helper Functions
//////////////////////////////////////////////////

//////////////////////////////////////////////////
/// Member Functions
//////////////////////////////////////////////////
void SceneLoader::UnloadCurrentScene() {
	Registry->ClearEntities();
}

void SceneLoader::LoadScene(Scenes level) {
	switch (level) {
	case StartMenu: {
			//---//EntityName//---//
			//Entity entity = Registry->CreateEntity();
			//entity.AddComponent<ComponentType>(args);

			break;
		}
	case Gameplay:{

			break;
		}
	case DeathScreen: {

			break;
		}

	}
}
