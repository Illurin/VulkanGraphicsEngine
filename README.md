# VulkanGraphicsEngine
基于Vulkan的封装引擎框架

## 重构之后的封装引擎框架

已完成：MeshRenderer和GameObject的封装，Skybox的开启，拆分描述符的管理

待完成：SkinnedMeshRenderer，ParticleSystem，PostProcessing的封装，以及和GameObject对象的关联，Model的封装

待改善：Pipeline的管理，GameObject的创建和材质的绑定，VkApp的结构

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
获取场景中的物件：
```
scene.GetGameObject("sphere")；
```
此结构会修改

修改Start方法和Update方法

## 天空盒和阴影贴图
设定场景的天空盒：
```
scene.SetSkybox(L"Assets\\skybox.png", 0.5f, 8);
```
第一个参数为图片的路径，后两个参数为球体的细分参数

设定场景的阴影图：
```
scene.SetShadowMap(width, height, lightDirection, radius);
```

## 加载图片类
使用WIC加载图片：
```
LoadPixelWithWIC(texturePath[i].c_str(), 32, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
```
使用WIC加载立方体图：
```
LoadCubeMapWithWIC(texturePath[i].c_str(), 32, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
```
使用STB加载图片：
```
LoadPixelWithSTB(texturePath[i].c_str(), 32, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
```
