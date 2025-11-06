#pragma once
#include <algorithm>
#include <utility>
#include <unordered_set>
#include <SDL.h>
#include "../ECS/ECS.h"
#include "../Components/ColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../EventSystem/EventBus.hpp"
#include "../EventSystem/Events/CollisionEvent.h"
#include "../EventSystem/Events/CollisionEnterEvent.h"
#include "../EventSystem/Events/CollisionExitEvent.h"

//NOTE: Change circle collider from taking in the radius to taking in the diameter to simplify this and the debug system

struct AABB { double minX, maxX, minY, maxY; };

static inline AABB BuildAABB(const Entity& e) {
    const auto& tr = e.GetComponent<TransformComponent>();
    const auto& c  = e.GetComponent<ColliderComponent>();

    if (c.type == Collider::Box) {
        const double x0 = tr.position.x + c.offset.x;
        const double y0 = tr.position.y + c.offset.y;
        return { x0, x0 + c.width_2r, y0, y0 + c.height };
    } else { // Circle: width_2r = diameter
        const double cx = tr.position.x + c.offset.x;
        const double cy = tr.position.y + c.offset.y;
        const double r  = c.width_2r * 0.5;
        return { cx - r, cx + r, cy - r, cy + r };
    }
}

static inline bool OverlapY(const AABB& a, const AABB& b) {
    return !(a.maxY < b.minY || b.maxY < a.minY);
}

class CollisionSystem : public System {
public:
	CollisionSystem() {
		RequireComponent<ColliderComponent>();
		RequireComponent<TransformComponent>();
		collidingPairs.reserve(VECTOR_INIT);
	}

	//For box colliders position of the entity + collider offset is the upper left corner
	//For circle colliders position of the entity + collider offset is the middle of the circle

	//https://leanrada.com/notes/sweep-and-prune
	//https://leanrada.com/notes/sweep-and-prune-2
	void Update(EventBus& eventBus) {
		auto& entities = GetSystemEntities();
		for (auto e : entities) e.GetComponent<ColliderComponent>().inCollision = false;

		// Build AABBs once
		const size_t n = entities.size();
		std::vector<AABB> aabbs; aabbs.reserve(n);
		for (auto e : entities) aabbs.emplace_back(BuildAABB(e));

		// Indices sorted by minX (sweep axis)
		std::vector<size_t> idx(n);
		for (size_t i = 0; i < n; ++i) idx[i] = i;
		std::sort(idx.begin(), idx.end(), [&](size_t i, size_t j){ return aabbs[i].minX < aabbs[j].minX; });

		// Active set holds indices with maxX >= current.minX
		std::vector<size_t> active; active.reserve(64);

		// Narrowphase + events
		for (size_t k = 0; k < n; ++k) {
			const size_t i = idx[k];
			const AABB& aAABB = aabbs[i];

			// prune old from active (no longer overlap on X)
			size_t write = 0;
			for (size_t read = 0; read < active.size(); ++read) {
				if (aabbs[active[read]].maxX >= aAABB.minX)
					active[write++] = active[read];
			}
			active.resize(write);

			// test against active set (X-overlap guaranteed), cull by Y
			Entity a = entities[i];
			auto& aColl = a.GetComponent<ColliderComponent>();

			for (size_t aj = 0; aj < active.size(); ++aj) {
				size_t j = active[aj];
				const AABB& bAABB = aabbs[j];
				if (!OverlapY(aAABB, bAABB)) continue; // 2nd axis prune

				Entity b = entities[j];
				auto& bColl = b.GetComponent<ColliderComponent>();

				if (CheckCollision(a, b)) {
					aColl.inCollision = true;
					bColl.inCollision = true;
					auto p = MakeEntityPair(a, b);
					if (!collidingPairs.contains(p)) {
						collidingPairs.insert(p);
						eventBus.EmitEvent<CollisionEnterEvent>(a, b);
					}
					eventBus.EmitEvent<CollisionEvent>(a, b);
				}
			}

			// insert current into active
			active.push_back(i);
		}

		// Collision exit sweep
		for (auto it = collidingPairs.begin(); it != collidingPairs.end();) {
			if (CheckCollision(it->first, it->second)) { ++it; }
			else {
				eventBus.EmitEvent<CollisionExitEvent>(it->first, it->second);
				it = collidingPairs.erase(it);
			}
		}
	}

	bool CheckCollision(const Entity& a, const Entity& b) {
		const TransformComponent& aTrns = a.GetComponent<TransformComponent>();
		const ColliderComponent& aColl = a.GetComponent<ColliderComponent>();
		const TransformComponent& bTrns = b.GetComponent<TransformComponent>();
		const ColliderComponent& bColl = b.GetComponent<ColliderComponent>();

		Collider aType = aColl.type;
		Collider bType = bColl.type;

		switch(aType) {
			//AABB Collision
			case Collider::Box: {
				if(bType == Box) {
					double aX = aTrns.position.x + aColl.offset.x;
					double aY = aTrns.position.y + aColl.offset.y;
					double bX = bTrns.position.x + bColl.offset.x;
					double bY = bTrns.position.y + bColl.offset.y;

					return (
						aX < bX + bColl.width_2r and
						aX + aColl.width_2r > bX and
						aY < bY + bColl.height and
						aY + aColl.height > bY
					);
				} 
				//b is Circle
				return CircleAndBoxCollision(b,a);
			} break;

			//Circle-Circle Collsion
			case Collider::Circle: {
				if(bType == Circle) {
					glm::vec2 aCenter = aTrns.position + aColl.offset;
					glm::vec2 bCenter = bTrns.position + bColl.offset;
					float combinedR = (aColl.width_2r + bColl.width_2r)/2.0;

					return glm::distance(aCenter, bCenter) < combinedR;
				}
				//a is Circle
				return CircleAndBoxCollision(a,b);
			} break;

			default: Logger::Err("Erronous trigger of the collision system");
			return false;
		}
	}

	//https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
	bool CircleAndBoxCollision(const Entity& a, const Entity& b) {
		TransformComponent& aTrns = a.GetComponent<TransformComponent>();
		ColliderComponent& aColl = a.GetComponent<ColliderComponent>();
		TransformComponent& bTrns = b.GetComponent<TransformComponent>();
		ColliderComponent& bColl = b.GetComponent<ColliderComponent>();

		glm::vec2 circleCenter = aTrns.position + aColl.offset;
		glm::vec2 boxHalfExtends = glm::vec2(bColl.width_2r/2.0, bColl.height/2.0);
		glm::vec2 boxCenter = bTrns.position + bColl.offset + boxHalfExtends;

		glm::vec2 difference = circleCenter - boxCenter;
		glm::vec2 clamped = glm::clamp(difference, -boxHalfExtends, boxHalfExtends);
		glm::vec2 closest = boxCenter + clamped;
		difference = closest - circleCenter;

		return glm::length(difference) < aColl.width_2r/2.0;
	}

	std::pair<Entity,Entity> MakeEntityPair(Entity a, Entity b) {
		if(a.id < b.id) return {a,b};
		return {b,a};
	}

	struct EntityPairHash {
		std::size_t operator()(const std::pair<Entity,Entity>& p) const noexcept {
			return std::hash<int>{}(p.first.id) ^ (std::hash<int>{}(p.second.id) + 0x9e3779b9 + (std::hash<int>{}(p.first.id) << 6) + (std::hash<int>{}(p.first.id) >> 2));
		}
	};

	struct EntityPairEq {
		bool operator()(const std::pair<Entity,Entity>& a,
				  const std::pair<Entity,Entity>& b) const noexcept {
			return a.first.id == b.first.id && a.second.id == b.second.id;
		}
	};

	std::unordered_set<std::pair<Entity, Entity>, EntityPairHash, EntityPairEq> collidingPairs;
};
