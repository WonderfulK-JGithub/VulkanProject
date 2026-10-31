#pragma once

#include "Engine.h"
#include "Vulkan/VkScene.h"

class App
{
public:
	App(int aWidth, int aHeight);
	~App();

	void Run();

private:
	Engine* myVulkanEngine;
	GLFWwindow* myWindow;
	VkScene* myVulkanScene;

	

	void BuildGLFWWindow(int aWidth, int aHeight);
	
};

