#pragma once 

enum Scenes{
	StartMenu,
	Gameplay,
	DeathScreen,
};

class SceneLoader {
public:
	void LoadScene(Scenes level);
	void UnloadCurrentScene();
private:
};

