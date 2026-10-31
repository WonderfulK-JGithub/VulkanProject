#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <vulkan/vulkan.hpp>

#include "imgui/imgui.h"

//#define DONT_USE_IMGUI

struct GLFWwindow;

class Engine;

struct ImGuiFonts
{
	ImFont* defaultFont;
	ImFont* defaultFontLarge;
	ImFont* iconFont;
	ImFont* iconFontLarge;
	ImFont* iconFontSmall;
};

class ImGUIRenderer {
public:
	ImGUIRenderer() {}

	~ImGUIRenderer() {}

	void Init(GLFWwindow* aWindow, Engine& aEngine);

	void Destroy();

	// Called once in every frame where the GUI is visible. Returns
	// a command buffer to be submitted into the queue.
	vk::CommandBuffer PrepareCommandBuffer(int Image);

	void BeginFrame();

	void EndFrame(vk::CommandBuffer aCommandBuffer);
	void Present();
private:

	void CreateDescriptorPool();

	void InitImGUI();

	GLFWwindow* myWindow;
	Engine* myEngine;
	ImFontAtlas* myFontAtlas;
	ImGuiFonts myFonts;

	//std::vector<vk::CommandBuffer> m_cmdBufs;
	vk::DescriptorPool m_descriptorPool;
};