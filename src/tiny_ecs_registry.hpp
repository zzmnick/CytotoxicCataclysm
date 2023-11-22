#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Transform> transforms;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec4> colors;
	ComponentContainer<Region> regions;
	ComponentContainer<Health> healthValues;
	ComponentContainer<PlayerHealthbar> healthbar;
	ComponentContainer<Weapon> weapons;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Invincibility> invincibility;
	ComponentContainer<Dash> dashes;
	ComponentContainer<Animation> animations;
	ComponentContainer<NoRotate> noRotates;
	ComponentContainer<CollidePlayer> collidePlayers;
	ComponentContainer<CollideEnemy> collideEnemies;
	ComponentContainer<PlayerBelonging> playerBelongings;
	ComponentContainer<Camera> camera;
	ComponentContainer<Cyst> cysts;
	ComponentContainer<TimedEvent> timedEvents;
	ComponentContainer<MenuElem> menuElems;
	ComponentContainer<MenuButton> menuButtons;
	

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&transforms);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&regions);
		registry_list.push_back(&healthValues);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&invincibility);
		registry_list.push_back(&animations);
		registry_list.push_back(&dashes);
		registry_list.push_back(&noRotates);
		registry_list.push_back(&weapons);
		registry_list.push_back(&collidePlayers);
		registry_list.push_back(&collideEnemies);
		registry_list.push_back(&playerBelongings);
		registry_list.push_back(&camera);
		registry_list.push_back(&cysts);
		registry_list.push_back(&timedEvents);
		registry_list.push_back(&menuElems);
		registry_list.push_back(&menuButtons);

	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;