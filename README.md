# VulkanGraphicsEngine
基于Vulkan的封装引擎框架

## 重构之后的封装引擎框架

已完成：MeshRenderer和GameObject的封装

待完成：SkinnedMeshRenderer，ParticleSystem，Skybox，PostProcessing的封装，以及和GameObject对象的关联，Model的封装

待改善：Pipeline的管理，描述符的管理，GameObject的创建和材质的绑定，VkApp的结构

存在bug待修复

## 语法结构
创建物体对象：
```
GameObject* sphere = scene.CreateGeosphere("sphere", 0.5f, 8);
sphere->transform.position = glm::vec3(0.0f, 1.0f, 1.0f);
```
添加材质至场景：
```
scene.CreateMaterial("sphere", &textures[0], glm::mat4x4(1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.05f, 0.05f, 0.05f), 0.8f);
scene.CreateMaterial("brick", &textures[1], glm::mat4x4(1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 0.01f);
scene.CreateMaterial("skybox", &textures[4], glm::mat4x4(1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f);
```
绑定物体与材质：
```
scene.BindMaterial("sphere", "sphere");
scene.BindMaterial("plane", "brick");
scene.BindMaterial("skybox", "skybox");
```

修改Start方法和Update方法
