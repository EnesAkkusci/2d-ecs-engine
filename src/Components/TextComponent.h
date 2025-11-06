#pragma once 
#include "SDL_pixels.h"
#include <string>
#include <SDL.h>
#include <glm/glm.hpp>

enum TextAlignment{
	TopLeft,
	TopCenter,
	TopRight
};

struct TextComponent {
	std::string text;
	std::string assetID;
	SDL_Color color;
	TextAlignment alignment;
	bool isFixed;

	TextComponent(
		std::string _text = "lorem ipsum",
		std::string _assetID = "",
		SDL_Color _color = {0, 0, 0},
		TextAlignment _alignment = TopLeft,
		bool _isFixed = true
	) :
		text(_text),
		assetID(_assetID),
		color(_color),
		alignment(_alignment),
		isFixed(_isFixed)
	{}
};
