#pragma once
#include "Scene.h"

class Editor {
public:
	Editor(Scene* scene) {
		this->scene = scene;
		objectSelected.resize(scene->GetObjectCount());
	}
	~Editor() {}

	void Update() {
		ImGui::SetNextWindowSize(ImVec2(300, scene->vkInfo->height), 0);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::Begin("Hierarchy");

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

		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(300, 500), 0);
		ImGui::SetNextWindowPos(ImVec2(scene->vkInfo->width - 300, 0));
		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::Begin("Inspector");

		if (currentObject) {
			ImGui::Text(("Name : " + currentObject->name).c_str());

			ImGui::Text("Transform : ");
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

	Scene* scene;

	int index = 0;
	std::vector<bool> objectSelected;
	GameObject* currentObject = nullptr;
};