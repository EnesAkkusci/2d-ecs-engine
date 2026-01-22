#pragma once
#include <deque>
#include <memory>
#include <bitset>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <algorithm>
#include "../Utils.hpp"
#include "../Logger/Logger.h"
#include "../Utils.hpp"

using Signature = std::bitset<MAX_COMPONENTS>;

////////////////////////////////////////////
//Entity
////////////////////////////////////////////
class Entity {
public:
	Entity(int _id) : id(_id) {};
	void Kill();

	int id;

	template <typename TComponent, typename ...TArgs>
	void AddComponent(TArgs&& ...args);
	template<typename TComponent>
	void RemoveComponent();
	template<typename TComponent>
	bool HasComponent() const;
	template<typename TComponent>
	TComponent& GetComponent() const;

	bool operator== (const Entity other) const { return id == other.id;}
	bool operator!= (const Entity& other) const { return id != other.id; }
	bool operator> (const Entity& other) const { return id > other.id; }
	bool operator< (const Entity& other) const { return id < other.id; }

	void AddTag(Tag tag);
	bool HasTag(Tag tag);
};

////////////////////////////////////////////
//Component
////////////////////////////////////////////

//Other than the data they carry Components only require an id per class level for Signature and storage access purposes.

class IComponent {
protected:
	static int nextID;
};

template<typename TComponent>
class Component : public IComponent{
public:
	static int GetID() {
		//Static local var. initializations only run in the first call of a function
		static const int id = nextID++;
		return id;
	}
};

////////////////////////////////////////////
//System
////////////////////////////////////////////
class System {
public:
	System() = default;
	virtual ~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector<Entity>& GetSystemEntities();
	const Signature& GetSystemComponentSignature() const;
	template <typename TComponent> void RequireComponent();

private:
	Signature systemComponentSignature;
	std::vector<Entity> systemEntities;
};

////////////////////////////////////////////
//Registry
////////////////////////////////////////////

class IPool {
public:
	virtual ~IPool() = default;
};

template<typename TComponent>
class Pool : public IPool{
public:
	Pool() {componentData.reserve(VECTOR_INIT);}
	~Pool() override = default;

	bool isEmpty() { return componentData.empty(); }
	int GetSize() const { return componentData.size(); }
	void Resize(int n) { componentData.resize(n); }
	void Clear() { componentData.clear(); }
	void Add(TComponent object) { componentData.push_back(object); }
	void Set(int index,TComponent object) { componentData[index] = object; }
	TComponent& Get(int index) { return componentData[index]; }
	TComponent& operator[] (int index) { return componentData[index]; }

private:
	std::vector<TComponent> componentData;
};

class Registry {
public:
	Registry();
	~Registry();
	static Registry* registry;
	void Update();

	Entity CreateEntity();
	void KillEntity(Entity entity);
	void ClearEntities();

	//Entity management
	//Add the entity to the systems with matching signatures
	void AddEntityToSystems(Entity entity);
	void RemoveEntityFromSystems(Entity entity);
	void ClearEntityFromCollidingPairs(Entity entity);

	//Component management
	template <typename TComponent, typename ...TArgs>
	void AddComponent(Entity entity, TArgs&& ...args);
	template<typename TComponent>
	void RemoveComponent(Entity entity);
	template<typename TComponent>
	bool HasComponent(Entity entity) const;
	template<typename TComponent>
	TComponent& GetComponent(Entity entity);

	//System management
	template<typename TSystem, typename ...TArgs>
	void AddSystem(TArgs&& ...args);
	template<typename TSystem>
	void RemoveSystem();
	template<typename TSystem>
	bool HasSystem() const;
	template<typename TSystem>
	TSystem& GetSystem() const;

	//Tag management
	void AddTagToEntity(Entity entity, Tag tag);
	bool EntityHasTag(Entity entity, Tag tag);
	std::vector<Entity>* u_GetEntitiesWithTag(Tag tag);

private:
	int numOfEntites = 0;
	//vector index == component type id, pool index == entity id
	//[][] would first go to the data pool belonging to a component type
	//and access the specific data of that component type belonging to an entity
	std::vector<std::shared_ptr<IPool>> componentPools;
	// index = entityID
	// vector keeping track of which components each entity "has"
	std::vector<Signature> entityComponentSignatures;
	//const time access to System object of the given type
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
	std::vector<Entity> entitesToBeAdded;
	std::vector<Entity> entitiesToBeKilled;
	std::deque<int> freeIDs;

	std::unordered_map<int, std::bitset<MAX_TAGS>> entityTagSignatures;
	std::unordered_map<Tag, std::vector<Entity>> tagToEntityMap;
};

////////////////////////////////////////////
//Entity templated functions
////////////////////////////////////////////

template<typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
	Registry::registry->AddComponent<TComponent>(*this,std::forward<TArgs>(args)...);
}

template<typename TComponent>
void Entity::RemoveComponent() {
	Registry::registry->RemoveComponent<TComponent>(*this);
}

template<typename TComponent>
bool Entity::HasComponent() const {
	return Registry::registry->HasComponent<TComponent>(*this);
}

template<typename TComponent>
TComponent& Entity::GetComponent() const {
	return Registry::registry->GetComponent<TComponent>(*this);
}

////////////////////////////////////////////
//System templated functions
////////////////////////////////////////////
template <typename TComponent>
void System::RequireComponent() {
	const int componentID = Component<TComponent>::GetID();
	systemComponentSignature.set(componentID);
}

////////////////////////////////////////////
//Registry templated functions
////////////////////////////////////////////
template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const int componentID = Component<TComponent>::GetID();
	const int entityID = entity.id;

	//This is basically only a consideration if the Registry is adding this component to a entity for the first time
	if (componentID >= componentPools.size()) {
		componentPools.resize(componentID + 1, nullptr);
	}
	if (!componentPools[componentID]) {
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentID] = newComponentPool;
	}

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);

	if (entityID >= componentPool->GetSize()) componentPool->Resize(numOfEntites);

	TComponent newComponent(std::forward<TArgs>(args)...);
	componentPool->Set(entityID, newComponent);
	entityComponentSignatures[entityID].set(componentID);
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const int componentID = Component<TComponent>::GetID();
	entityComponentSignatures[entity.id].set(componentID, false);
}

template<typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const int componentID = Component<TComponent>::GetID();
	return entityComponentSignatures[entity.id].test(componentID);
}

template<typename TComponent>
TComponent& Registry::GetComponent(Entity entity) {
	auto it = std::find(freeIDs.begin(), freeIDs.end(), entity.id);
	if(it != freeIDs.end()) {
		Logger::Err("Trying to get a free id's component:" + std::to_string(entity.id));
	}
	Logger::Assert(entity.HasComponent<TComponent>(), 
				"Entity with id: " + std::to_string(entity.id) 
				+ " does not have component: " + typeid(TComponent).name());

	const int componentID = Component<TComponent>::GetID();
	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);
	return componentPool->Get(entity.id);
}

// template<typename TComponent>
// TComponent& Registry::GetComponent(Entity entity) {
//     Logger::Assert(entity.HasComponent<TComponent>(), 
//         "Entity " + std::to_string(entity.id) + " missing component: " + typeid(TComponent).name());
//
//     const int componentID = Component<TComponent>::GetID();
//
//     Logger::Assert(componentID < componentPools.size() && componentPools[componentID] != nullptr,
//         "Invalid pool for component: " + std::string(typeid(TComponent).name()));
//
//     auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentID]);
//     return componentPool->Get(entity.id);
// }

template<typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<System> newSystem = std::make_shared<TSystem>(std::forward<TSystem>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template<typename TSystem>
void Registry::RemoveSystem() { 
	auto system = systems.find(std::type_index(typeid(TSystem)));
	if (system != systems.end()) systems.erase(system);
}

template<typename TSystem>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template<typename TSystem>
TSystem& Registry::GetSystem() const { 
	auto system = systems.find(std::type_index(typeid(TSystem)));
	if(system == systems.end()) Logger::Err("Non existing system access attempt.");
	return *(std::static_pointer_cast<TSystem>(system->second));
}
