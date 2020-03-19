#pragma once
#include "Component.h"

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	float eulerAngle;
	float localEulerAngle;
};

struct GameObject {
	std::string name;

	GameObject* parent;
	std::vector<GameObject*> children;

	Transform transform;
};

class Scene {
public:
	//Create·½·¨
	GameObject* CreatePlane();
	GameObject* CreateSphere();

private:
	std::unordered_map<std::string, GameObject> gameObjects;
	std::unordered_map<std::string, Material> materials;
};
