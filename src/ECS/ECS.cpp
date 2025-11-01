#include <string>
#include <algorithm>
#include <typeindex>
#include "ECS.h"
#include "../Systems/RenderSystem.hpp"
#include "../Systems/CollisionSystem.hpp"

Registry* Registry::registry = nullptr;

//ID to be assigned to the first Component that calls GetID()
int IComponent::nextID = 0;

////////////////////////////////////////////
//Entity
////////////////////////////////////////////

void Entity::Kill() {
	Registry::registry->KillEntity(*this);
}

void Entity::AddTag(Tag tag) {
	Registry::registry->AddTagToEntity(*this, tag);
}

bool Entity::HasTag(Tag tag) {
	return Registry::registry->EntityHasTag(*this, tag);
};

////////////////////////////////////////////
//Component
////////////////////////////////////////////

////////////////////////////////////////////
//System
////////////////////////////////////////////

void System::AddEntityToSystem(Entity entity) {
	systemEntities.emplace_back(entity.id);
}

void System::RemoveEntityFromSystem(Entity entity) {
	auto it = std::find(systemEntities.begin(), systemEntities.end(), entity);
	if (it != systemEntities.end()) systemEntities.erase(it);
}

std::vector<Entity>& System::GetSystemEntities() {
	return systemEntities;
}

const Signature& System::GetSystemComponentSignature() const {
	return systemComponentSignature;
}

////////////////////////////////////////////
//Registry
////////////////////////////////////////////

Registry::Registry() {
	//Pre-allocating vector memories
	componentPools.reserve(MAX_COMPONENTS);
	entityComponentSignatures.resize(VECTOR_INIT);
	entitesToBeAdded.reserve(10);
	entitiesToBeKilled.reserve(10);

	registry = this;
}

Registry::~Registry() {
	if (registry == this) registry = nullptr;
}

void Registry::Update() {
	for (auto entity : entitiesToBeKilled) {
		RemoveEntityFromSystems(entity);
		entityComponentSignatures[entity.id].reset();
		entityTagSignatures[entity.id].reset();
		ClearEntityFromCollidingPairs(entity);
		freeIDs.push_back(entity.id);
	}
	entitiesToBeKilled.clear();

	for (auto entity : entitesToBeAdded) {
		AddEntityToSystems(entity);
	}
	entitesToBeAdded.clear();
}

Entity Registry::CreateEntity() {
	int entityID;

	if(freeIDs.empty()) {
		entityID = numOfEntites++;
		if (entityID >= static_cast<int>(entityComponentSignatures.size())) {
			entityComponentSignatures.resize(entityID + (entityID / 2));
		}
	} else {
		entityID = freeIDs.front();
		freeIDs.pop_front();
	}

	// Logger::Log("Entity created with id " + std::to_string(entityID));

	Entity entity(entityID);
	entitesToBeAdded.emplace_back(entity);
	return entity;
}

void Registry::KillEntity(Entity entity) {
	entitiesToBeKilled.push_back(entity);
}

// void Registry::ClearEntities() {
// 	for (int i = 0; i < numOfEntites; i++) {
// 		entitiesToBeKilled.emplace_back(i);
// 	}
// }

void Registry::ClearEntities() {
    // assume entityComponentSignatures.size() == max possible entity ID count
    for (int id = 0; id < (int)entityComponentSignatures.size(); ++id) {
        // if this slot actually has any components, it’s “alive”
        if (entityComponentSignatures[id].any()) {
            entitiesToBeKilled.emplace_back(id);
        }
    }
}

void Registry::AddEntityToSystems(Entity entity) {
	const Signature entitySignature = entityComponentSignatures[entity.id];

	auto renderSign = systems[std::type_index(typeid(RenderSystem))]->GetSystemComponentSignature();

	for (auto& system : systems) {
		const auto& systemSignature = system.second->GetSystemComponentSignature();
		if ((systemSignature & entitySignature) == systemSignature) system.second->AddEntityToSystem(entity);

		if ((renderSign & systemSignature) == renderSign) {
			std::vector<Entity>& refVector = system.second->GetSystemEntities();
			std::sort(refVector.begin(), refVector.end(), [](const Entity &a, const Entity &b) {
				return a.GetComponent<SpriteComponent>().zIndex < b.GetComponent<SpriteComponent>().zIndex;
			});
		}
	}
}

void Registry::RemoveEntityFromSystems(Entity entity) {
	for (auto& system : systems) system.second->RemoveEntityFromSystem(entity);
}

void Registry::ClearEntityFromCollidingPairs(Entity entity) {
	auto& set = GetSystem<CollisionSystem>().collidingPairs;
	for (auto it = set.begin(); it != set.end();) {
		if(it->first.id == entity.id or it->second.id == entity.id) {
			it = set.erase(it);
		} else {
			++it;
		}
	}
}

void Registry::AddTagToEntity(Entity entity,Tag tag) {
	entityTagSignatures[entity.id].set(tag);
	tagToEntityMap[tag].push_back(entity);
}

bool Registry::EntityHasTag(Entity entity, Tag tag) {
	return entityTagSignatures[entity.id].test(tag);
}

std::vector<Entity>* Registry::u_GetEntitiesWithTag(Tag tag) {
	auto it = tagToEntityMap.find(tag);
	if (it == tagToEntityMap.end()) {
		Logger::Err("No entity has this tag: " + std::to_string(tag));
		return nullptr;
	} else {
		return &it->second;
	}
}
