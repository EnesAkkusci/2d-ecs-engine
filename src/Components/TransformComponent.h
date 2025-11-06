#pragma once
#include <glm/glm.hpp>

struct TransformComponent {
	TransformComponent(
		glm::vec2 _position = glm::vec2(0),
		glm::vec2 _scale = glm::vec2(1),
		double _rotation = 0,
		bool _hasLabel = false
	)
		: position(_position), scale(_scale), rotation(_rotation), hasLabel(_hasLabel){};

	glm::vec2 position;
	glm::vec2 scale;
	double rotation;
	bool hasLabel;
};
