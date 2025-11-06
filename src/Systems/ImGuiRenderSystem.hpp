#pragma once 
#include <SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"

class ImGuiRenderSystem : public System {
public:
	ImGuiRenderSystem() = default;

	void Update(SDL_Renderer* renderer, SDL_Rect& camera, Registry& registry, double deltaTime) {
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// if(ImGui::Begin("Blue man teleporter")) {
		// 	static float xPos;
		// 	static float yPos;
		// 	ImGui::InputFloat("x position", &xPos);
		// 	ImGui::InputFloat("y position", &yPos);
		//
		// 	if(ImGui::Button("Teleport blue man")) {
		// 		std::vector<Entity>* entities = registry.u_GetEntitiesWithTag(Tag::Player);
		// 		if(entities) {
		// 			for (auto entity : *entities) {
		// 				auto& transform = entity.GetComponent<TransformComponent>();
		//
		// 				transform.position.x = xPos;
		// 				transform.position.y = yPos;
		// 			}
		// 		}
		// 	}
		//
		// 	static int playerSpeed;
		// 	ImGui::InputInt("Player speed", &playerSpeed);
		//
		// 	if(ImGui::Button("Change speed")) {
		// 		std::vector<Entity>* entities = registry.u_GetEntitiesWithTag(Tag::Player);
		// 		if(entities) {
		// 			for (auto entity : *entities) {
		// 				auto& pControl = entity.GetComponent<PlayerControlComponent>();
		//
		// 				pControl.playerSpeed = playerSpeed;
		// 			}
		// 		}
		// 	}
		// }
		// ImGui::End(); //Comment out for demo window

		// if(ImGui::Begin("Blue Man")){
		// 	if(ImGui::Button("Kill blue man")) {
		// 		std::vector<Entity>* entities = registry.u_GetEntitiesWithTag(Player);
		// 		if(entities) {
		// 			for (Entity e : *entities) {
		// 				e.Kill();
		// 			}
		// 		}
		// 	}
		// }
		// ImGui::End();

		for (auto entity : GetSystemEntities()) {
			//Maybe add check to make sure this id is not in freeIds
			if (!entity.HasComponent<TransformComponent>()) continue;
			if (!entity.GetComponent<TransformComponent>().hasLabel) continue;

			const auto& transform = entity.GetComponent<TransformComponent>();

			// Convert world → screen
			float screenX = transform.position.x - camera.x;
			float screenY = transform.position.y - camera.y;

			// Position next ImGui window
			ImGui::SetNextWindowPos(ImVec2(screenX, screenY), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
			ImGui::SetNextWindowBgAlpha(0.3f); // transparent background
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration 
				| ImGuiWindowFlags_AlwaysAutoResize 
				| ImGuiWindowFlags_NoSavedSettings 
				| ImGuiWindowFlags_NoNav 
				| ImGuiWindowFlags_NoMove;

			std::string label = "EntityID_" + std::to_string(entity.id);
			if (ImGui::Begin(label.c_str(), NULL, flags)) {
				ImGui::Text("ID: %d", entity.id);
			}
			ImGui::End();
		}

		// Display a small overlay window to display the map position using the mouse
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav;
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("Map coordinates", NULL, windowFlags)) {
			ImGui::Text(
				"Mouse map coordinates (x=%.1f, y=%.1f)",
				ImGui::GetIO().MousePos.x + camera.x,
				ImGui::GetIO().MousePos.y + camera.y
			);
			ImGui::Text(
				"FPS: %.1f", ImGui::GetIO().Framerate);
		}
		ImGui::End();
	
		// ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
	}
};
