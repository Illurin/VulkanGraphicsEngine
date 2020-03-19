#include "ParticleSystem.h"

float ParticleSystem::Random(float range) {
	std::uniform_real_distribution<float> rndDist(0.0f, range);
	return rndDist(randomEngine);
}

void ParticleSystem::SetParticleProperty(Property property) {
	this->property = property;
}

void ParticleSystem::SetEmitterProperty(Emitter emitter) {
	this->emitter = emitter;
}

void ParticleSystem::SetTextureProperty(Texture texture) {
	this->texture = texture;
}

void ParticleSystem::SetSubParticle(SubParticle subParticleProperty) {
	this->subParticleProperty = subParticleProperty;
}

void ParticleSystem::InitParticles(Particle* particle) {
	particle->velocity = glm::vec3(property.minVelocity.x + Random(property.maxVelocity.x - property.minVelocity.x),
		property.minVelocity.y + Random(property.maxVelocity.y - property.minVelocity.y), property.minVelocity.z + Random(property.maxVelocity.z - property.minVelocity.z));
	
	particle->color = glm::vec4(property.color, 0.0f);
	particle->color.a = property.minAlpha + Random(property.maxAlpha - property.minAlpha);
	particle->size = property.minSize + Random(property.maxSize - property.minSize);

	float theta = Random(2.0f * glm::pi<float>());
	float phi = Random(glm::pi<float>()) - glm::pi<float>() / 2.0f;
	float r = Random(emitter.radius);

	particle->position.x = r * cos(theta) * cos(phi);
	particle->position.y = r * sin(phi);
	particle->position.z = r * sin(theta) * cos(phi);

	particle->position += glm::vec3(emitter.position);
	particle->lastTime = property.minLastTime + Random(property.maxLastTime - property.minLastTime);
	particle->lifetime = 0.0f;

	particle->texCoord = glm::vec4(0.0f, 0.0f, 1.0f / (float)texture.splitX, 1.0f / (float)texture.splitY);
}

void ParticleSystem::InitSubParticle(uint32_t index) {
	auto& particle = subParticles[index];
	particle.color = glm::vec4(subParticleProperty.color, particles[index].color.a);
	particle.texCoord = glm::vec4(0.0f, 0.0f, 1.0f / subParticleProperty.texture.splitX, 1.0f / subParticleProperty.texture.splitY);
	particle.size = subParticleProperty.size;
	particle.lastTime = subParticleProperty.lastTime;
	particle.position = particles[index].position;
	particle.velocity = particles[index].velocity;
	particle.lifetime = 0.0f;
}

void ParticleSystem::PrepareParticles(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp) {
	particles.resize(emitter.maxParticleNum);
	for (auto& particle : particles) {
		InitParticles(&particle);
	}

	if (particleBuffer != nullptr)
		particleBuffer->DestroyBuffer(device);

	if (subParticleProperty.used)
		subParticles.resize(emitter.maxParticleNum);

	particleBuffer = std::make_unique<Buffer<Particle>>(device, subParticleProperty.used ? (emitter.maxParticleNum * 2) : emitter.maxParticleNum, vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);
}

void ParticleSystem::UpdateParticles(float deltaTime, vk::Device* device) {
	for (uint32_t i = 0; i < particles.size(); i++) {
		auto& particle = particles[i];
		particle.position += particle.velocity * deltaTime;
		particle.color.a -= property.colorFadeSpeed * deltaTime;
		particle.size -= property.sizeFadeSpeed * deltaTime;
		particle.size = particle.size >= 0.0f ? particle.size : 0.0f;
		particle.lifetime += deltaTime;

		uint32_t imageIndex = static_cast<uint32_t>(particle.lifetime / particle.lastTime * (float)texture.texCount);
		
		particle.texCoord.x = imageIndex % texture.splitX * particle.texCoord.z;
		particle.texCoord.y = imageIndex / texture.splitY * particle.texCoord.w;

		if (particle.lifetime >= particle.lastTime) {
			if (subParticleProperty.used)
				InitSubParticle(i);
			InitParticles(&particle);
		}
	}
	if (subParticleProperty.used) {
		for (uint32_t i = 0; i < subParticles.size(); i++) {
			auto& particle = subParticles[i];
			if (particle.size > 0.0f) {
				particle.position += particle.velocity * deltaTime;
				particle.color.a -= property.colorFadeSpeed * deltaTime;
				particle.size -= property.sizeFadeSpeed * deltaTime;
				particle.lifetime += deltaTime;

				uint32_t imageIndex = static_cast<uint32_t>(particle.lifetime / particle.lastTime * (float)texture.texCount);

				particle.texCoord.x = imageIndex % subParticleProperty.texture.splitX * particle.texCoord.z;
				particle.texCoord.y = imageIndex / subParticleProperty.texture.splitY * particle.texCoord.w;

				if (particle.lifetime >= particle.lastTime) {
					particle.size = 0.0f;
				}
			}
		}
	}
	particleBuffer->CopyData(device, 0, emitter.maxParticleNum, particles.data());
	if(subParticleProperty.used)
		particleBuffer->CopyData(device, emitter.maxParticleNum, emitter.maxParticleNum, subParticles.data());
}

void ParticleSystem::DrawParticles(vk::CommandBuffer* cmd) {
	const vk::DeviceSize offsets[1] = { 0 };
	cmd->bindVertexBuffers(0, 1, &particleBuffer->GetBuffer(), offsets);
	cmd->draw(emitter.maxParticleNum, 1, 0, 0);
}

void ParticleSystem::DrawSubParticles(vk::CommandBuffer* cmd) {
	const vk::DeviceSize offsets[1] = { 0 };
	cmd->bindVertexBuffers(0, 1, &particleBuffer->GetBuffer(), offsets);
	cmd->draw(emitter.maxParticleNum, 1, emitter.maxParticleNum, 0);
}