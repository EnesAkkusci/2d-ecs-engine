#pragma once
#include "../ECS/ECS.h"
#include "../EventSystem/EventBus.hpp"
#include "../EventSystem/Events/AnimationDoneEvent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/SpriteComponent.h"
#include <cassert>

class AnimationSystem : public System {
public:
	AnimationSystem() {
		RequireComponent<AnimationComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(double deltaTime, EventBus& eventBus) {
		for (auto entity : GetSystemEntities()) {
			auto& sprite = entity.GetComponent<SpriteComponent>();
			AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();
			auto& animation = animationComponent.currentAnimation;

			#ifdef DEBUG
			Logger::Assert(animation.frameCount > 0, "Animation with 0 frame count.");
			#endif

			animation.frameTimer -= deltaTime;
			if(animation.frameTimer <= 0) {
				//If we are in the last frame of the animation
				if(animation.currentFrame == animation.frameCount-1 && !animation.isLooping) {
					eventBus.EmitEvent<AnimationDoneEvent>(entity, animation.name);
				} else {
					sprite.srcRect.y = sprite.height * animation.sheetIndex;
					animation.currentFrame = (animation.currentFrame + 1 ) % animation.frameCount;
					sprite.srcRect.x = sprite.width * animation.currentFrame;
					animation.frameTimer = animation.secondsPerAnimationFrame;
				}
			}
		}
	}
};

inline void ChangeAnimation(Entity& entity, std::string_view name) {
	auto& animComp = entity.GetComponent<AnimationComponent>();
	auto& sprite = entity.GetComponent<SpriteComponent>();

	auto it = animComp.animations.find(name);
	assert(it != animComp.animations.end());

	const auto& nextAnimation = it->second;
	if(animComp.currentAnimation.sheetIndex == nextAnimation.sheetIndex) return; //same animation clip
	
	animComp.currentAnimation = nextAnimation;
	AnimationClip &ac = animComp.currentAnimation;
	ac.currentFrame = 0;
	ac.frameTimer = ac.secondsPerAnimationFrame;

	sprite.srcRect.y = sprite.height * ac.sheetIndex;
	sprite.srcRect.x = sprite.width  * ac.currentFrame;
};
