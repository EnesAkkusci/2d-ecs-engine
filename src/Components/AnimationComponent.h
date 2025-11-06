#pragma once
#include <initializer_list>
#include <unordered_map>
#include <string_view>

struct AnimationClip {
	AnimationClip(std::string_view name = "", int index = 0, int frameCount = 0, bool looping = true, int frameRate = 12)
	:
	name(name),
	sheetIndex(index),
	frameCount(frameCount),
	isLooping(looping),
	animationFrameRate(frameRate),
	secondsPerAnimationFrame(1.0f/animationFrameRate),
	frameTimer(secondsPerAnimationFrame),
	currentFrame(0)
	{} 

	std::string_view name;
	int sheetIndex;
	int frameCount;
	bool isLooping;
	int animationFrameRate;
	double secondsPerAnimationFrame;
	double frameTimer;
	int currentFrame;
};

struct AnimationComponent {
	AnimationComponent() = default;
	AnimationComponent(std::initializer_list<AnimationClip> init)
	{
		for(auto& clip : init) {
			animations.emplace(clip.name, clip);
		}
		currentAnimation = *init.begin();
	};

	std::unordered_map<std::string_view, AnimationClip> animations;
	AnimationClip currentAnimation;
};
