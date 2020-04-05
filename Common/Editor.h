#pragma once
#include "Scene.h"
#include <sstream>

class Editor {
public:
	Editor(Scene* scene) {
		this->scene = scene;
		objectSelected.resize(scene->GetObjectCount());
	}
	~Editor() {}

	void Update() {
		ImGui::SetNextWindowSize(ImVec2(300, scene->vkInfo->height - 300), 0);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::Begin("Hierarchy");

		const char* hierarchyTypeName[] = { "GameObject", "Material", "Light" };
		ImGui::Combo("Type", &hierarchyType, hierarchyTypeName, 3);

		switch (hierarchyType) {
		case 0:
			if (currentMaterial)
				currentMaterial = nullptr;

			index = 0;

			for (auto& gameObject : scene->GetRootObjects()) {
				if (ImGui::TreeNode(gameObject->name.c_str())) {
					if (!objectSelected[index])
						currentObject = gameObject;

					objectSelected[index] = true;
					index++;

					PrintChildren(gameObject);
					ImGui::TreePop();
				}
				else {
					if (objectSelected[index])
						currentObject = gameObject;

					objectSelected[index] = false;
					index += (gameObject->children.size() + 1);
				}
			}
			break;

		case 1:
			if (currentObject)
				currentObject = nullptr;

			for (auto& material : scene->GetAllMaterials()) {
				if (ImGui::Selectable(material->name.c_str()))
					currentMaterial = material;
			}
			break;

		case 2:
			for (uint32_t i = 0; i < NUM_DIRECTIONAL_LIGHT; i++) {
				std::stringstream name;
				name << i;

				if (ImGui::Selectable(("Directional light" + name.str()).c_str()))
					currentLightIndex = i;
			}
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			for (uint32_t i = 0; i < NUM_POINT_LIGHT; i++) {
				std::stringstream name;
				name << i;

				if (ImGui::Selectable(("Point light" + name.str()).c_str()))
					currentLightIndex = i + NUM_DIRECTIONAL_LIGHT;
			}
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			for (uint32_t i = 0; i < NUM_SPOT_LIGHT; i++) {
				std::stringstream name;
				name << i;

				if (ImGui::Selectable(("Spot light" + name.str()).c_str()))
					currentLightIndex = i + NUM_DIRECTIONAL_LIGHT + NUM_POINT_LIGHT;
			}
			break;
		}

		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(300, 300), 0);
		ImGui::SetNextWindowPos(ImVec2(0, scene->vkInfo->height - 300));
		ImGui::SetNextWindowBgAlpha(0.3f);

		ImGui::Begin("Menu");

		ImGui::Text("New game object");

		const char* objectTypes[] = { "Plane", "Geosphere" };
		ImGui::Combo("Object type", &newObject.currentType, objectTypes, 2);

		switch (newObject.currentType) {
		case 0:
			ImGui::InputFloat("plane width", &newObject.plane.width, 0.1f, 0.3f, 5);
			ImGui::InputFloat("plane depth", &newObject.plane.depth, 0.1f, 0.3f, 5);
			ImGui::InputInt("tex repeat x", &newObject.plane.texRepeatX, 1, 10);
			ImGui::InputInt("tex repeat y", &newObject.plane.texRepeatY, 1, 10);

			break;

		case 1:
			ImGui::InputFloat("sphere radius", &newObject.geosphere.radius, 0.1f, 0.3f, 5);
			ImGui::InputInt("sphere subdivisons", &newObject.geosphere.numSubdivisions, 1, 10);

			break;
		}

		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(300, 500), 0);
		ImGui::SetNextWindowPos(ImVec2(scene->vkInfo->width - 300, 0));
		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::Begin("Inspector");

		switch (hierarchyType) {
		case 0:
			if (currentObject) {
				ImGui::Text(("Name : " + currentObject->name).c_str());

				ImGui::Text("Transform : ");

				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::Text("Position:");
				ImGui::InputFloat("x", &currentObject->transform.position.x, 0.1f, 0.3f, 5);
				ImGui::InputFloat("y", &currentObject->transform.position.y, 0.1f, 0.3f, 5);
				ImGui::InputFloat("z", &currentObject->transform.position.z, 0.1f, 0.3f, 5);

				ImGui::Text("Scale : ");
				ImGui::InputFloat("x ", &currentObject->transform.scale.x, 0.1f, 0.3f, 5);
				ImGui::InputFloat("y ", &currentObject->transform.scale.y, 0.1f, 0.3f, 5);
				ImGui::InputFloat("z ", &currentObject->transform.scale.z, 0.1f, 0.3f, 5);

				ImGui::Text("Local euler angle : ");
				ImGui::InputFloat("x  ", &currentObject->transform.localEulerAngle.x, 0.1f, 0.3f, 5);
				ImGui::InputFloat("y  ", &currentObject->transform.localEulerAngle.y, 0.1f, 0.3f, 5);
				ImGui::InputFloat("z  ", &currentObject->transform.localEulerAngle.z, 0.1f, 0.3f, 5);

				ImGui::Text("Euler angle : ");
				ImGui::InputFloat("x   ", &currentObject->transform.eulerAngle.x, 0.1f, 0.3f, 5);
				ImGui::InputFloat("y   ", &currentObject->transform.eulerAngle.y, 0.1f, 0.3f, 5);
				ImGui::InputFloat("z   ", &currentObject->transform.eulerAngle.z, 0.1f, 0.3f, 5);

				if (ImGui::Button("Reset transform", ImVec2(200, 30))) {
					currentObject->transform.position = glm::vec3(0.0f);
					currentObject->transform.scale = glm::vec3(1.0f);
					currentObject->transform.localEulerAngle = glm::vec3(0.0f);
					currentObject->transform.eulerAngle = glm::vec3(0.0f);
				}

				currentObject->UpdateData();
			}
			break;

		case 1:
			if (currentMaterial) {
				ImGui::Text(("Name : " + currentMaterial->name).c_str());

				float diffuseAlbedo[4] = { currentMaterial->diffuseAlbedo.r,  currentMaterial->diffuseAlbedo.g, currentMaterial->diffuseAlbedo.b, currentMaterial->diffuseAlbedo.a };
				ImGui::ColorEdit4("Diffuse albedo", diffuseAlbedo);
				currentMaterial->diffuseAlbedo.r = diffuseAlbedo[0];
				currentMaterial->diffuseAlbedo.g = diffuseAlbedo[1];
				currentMaterial->diffuseAlbedo.b = diffuseAlbedo[2];
				currentMaterial->diffuseAlbedo.a = diffuseAlbedo[3];

				ImGui::Text("Fresnel R0 : ");
				ImGui::InputFloat("r", &currentMaterial->fresnelR0.r, 0.1f, 0.3f, 5);
				ImGui::InputFloat("g", &currentMaterial->fresnelR0.g, 0.1f, 0.3f, 5);
				ImGui::InputFloat("b", &currentMaterial->fresnelR0.b, 0.1f, 0.3f, 5);

				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::InputFloat("roughness", &currentMaterial->roughness, 0.01f, 0.3f, 5);

				if (ImGui::Button("Reset material", ImVec2(200, 30))) {
					currentMaterial->diffuseAlbedo = glm::vec4(1.0f);
					currentMaterial->fresnelR0 = glm::vec3(0.0f);
					currentMaterial->roughness = 0.0f;
				}

				currentMaterial->dirtyFlag = true;
			}
			break;

		case 2:
			Light* lights = scene->GetLights();
			Light* currentLight = &lights[currentLightIndex];

			float lightStrength[3] = { currentLight->strength.r,  currentLight->strength.g, currentLight->strength.b };
			ImGui::ColorEdit3("Strength", lightStrength);
			currentLight->strength.r = lightStrength[0];
			currentLight->strength.g = lightStrength[1];
			currentLight->strength.b = lightStrength[2];

			if (currentLightIndex < NUM_DIRECTIONAL_LIGHT) {
				ImGui::Text("Direction");
				ImGui::InputFloat("x", &currentLight->strength.x, 0.01f, 0.3f, 5);
				ImGui::InputFloat("y", &currentLight->strength.y, 0.01f, 0.3f, 5);
				ImGui::InputFloat("z", &currentLight->strength.z, 0.01f, 0.3f, 5);
			}
			else if (currentLightIndex < NUM_DIRECTIONAL_LIGHT + NUM_POINT_LIGHT) {
				ImGui::Text("Position");
				ImGui::InputFloat("x", &currentLight->position.x, 0.01f, 0.3f, 5);
				ImGui::InputFloat("y", &currentLight->position.y, 0.01f, 0.3f, 5);
				ImGui::InputFloat("z", &currentLight->position.z, 0.01f, 0.3f, 5);

				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::InputFloat("Fall off start", &currentLight->fallOffStart, 0.01f, 0.3f, 5);
				ImGui::InputFloat("Fall off start", &currentLight->fallOffEnd, 0.01f, 0.3f, 5);
			}
			else {
				ImGui::Text("Direction");
				ImGui::InputFloat("x", &currentLight->strength.x, 0.01f, 0.3f, 5);
				ImGui::InputFloat("y", &currentLight->strength.y, 0.01f, 0.3f, 5);
				ImGui::InputFloat("z", &currentLight->strength.z, 0.01f, 0.3f, 5);

				ImGui::Text("Position");
				ImGui::InputFloat("x ", &currentLight->position.x, 0.01f, 0.3f, 5);
				ImGui::InputFloat("y ", &currentLight->position.y, 0.01f, 0.3f, 5);
				ImGui::InputFloat("z ", &currentLight->position.z, 0.01f, 0.3f, 5);

				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::InputFloat("Fall off start", &currentLight->fallOffStart, 0.01f, 0.3f, 5);
				ImGui::InputFloat("Fall off end", &currentLight->fallOffEnd, 0.01f, 0.3f, 5);

				ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

				ImGui::InputFloat("Spot power", &currentLight->spotPower, 0.01f, 0.3f, 5);
			}

			break;
		}

		ImGui::End();
	}

private:
	void PrintChildren(GameObject* parent) {
		for (auto& child : parent->children) {
			if (ImGui::TreeNode(child->name.c_str())) {
				if (!objectSelected[index])
					currentObject = child;

				objectSelected[index] = true;
				index++;

				PrintChildren(child);
				ImGui::TreePop();
			}
			else {
				if (objectSelected[index])
					currentObject = child;

				objectSelected[index] = false;
				index += (child->children.size() + 1);
			}
		}
	}
	void CreateNewObject() {

	}

	Scene* scene;

	int index = 0;
	std::vector<bool> objectSelected;
	GameObject* currentObject = nullptr;
	Material* currentMaterial = nullptr;
	int currentLightIndex = 0;

	int hierarchyType = 0;

	struct {
		int currentType = 0;
		struct {
			float width = 0.0f;
			float depth = 0.0f;
			int texRepeatX = 0;
			int texRepeatY = 0;
		}plane;
		struct {
			float radius = 0.0f;
			int numSubdivisions = 0;
		}geosphere;
	}newObject;
};