#pragma once
#include "vkUtil.h"
#include "Texture.h"
#include <random>

class ParticleSystem {
public:
	struct Particle {
		glm::vec3 position;
		float size;
		glm::vec4 color;
		glm::vec4 texCoord;

		//not used in shader
		glm::vec3 velocity;
		float lastTime;
		float lifetime;
	};

	struct Property {
		float minSize;
		float maxSize;
		glm::vec3 minVelocity;
		glm::vec3 maxVelocity;
		float minLastTime;
		float maxLastTime;
		float maxAlpha;
		float minAlpha;
		glm::vec3 color;
		float colorFadeSpeed;
		float sizeFadeSpeed;
	};

	struct Texture {
		uint32_t splitX;
		uint32_t splitY;
		uint32_t texCount;
	};

	struct Emitter {
		uint32_t maxParticleNum;
		glm::vec3 position;
		float radius;
	};

	struct SubParticle {
		bool used = false;
		float lastTime;
		float size;
		Texture texture;
		glm::vec3 color;
	};

	void SetParticleProperty(Property property);
	void SetEmitterProperty(Emitter emitter);
	void SetTextureProperty(Texture texture);
	void SetSubParticle(SubParticle subParticleProperty);

	void PrepareParticles(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp);
	void UpdateParticles(float deltaTime, vk::Device* device);
	void DrawParticles(vk::CommandBuffer* cmd);
	void DrawSubParticles(vk::CommandBuffer* cmd);

private:
	void InitParticles(Particle* particle);
	void InitSubParticle(uint32_t index);

	std::unique_ptr<Buffer<Particle>> particleBuffer = nullptr;
	std::vector<Particle> particles;
	std::vector<Particle> subParticles;
	std::default_random_engine randomEngine;
	float Random(float range);

	/*Properties of the particle system*/
	Property property;
	Emitter emitter;
	Texture texture;
	SubParticle subParticleProperty;
};