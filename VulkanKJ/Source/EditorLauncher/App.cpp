#include "App.h"
#include <string>
#include <Input/KeyboardInput.h>
#include <Input/MouseInput.h>
#include <Shaders/Shaders.h>
#include <Util/Logging/Logging.h>

#include "../Editor/WonderfulEditor.h"

App::App(int aWidth, int aHeight)
{
	INIT_LOGGING();
	BuildGLFWWindow(aWidth, aHeight);

	myVulkanEngine = new Engine(aWidth, aHeight, myWindow);
	myVulkanScene = new VkScene();
	
	glfwSetCharCallback(myWindow, KeyboardInput::CharacterCallback);
	glfwSetKeyCallback(myWindow, KeyboardInput::KeyCallback);
	glfwSetCursorPosCallback(myWindow, MouseInput::CursorPosCallback);
	glfwSetMouseButtonCallback(myWindow, MouseInput::MouseInputCallback);
	glfwSetScrollCallback(myWindow, MouseInput::ScrollCallback);
}

App::~App()
{
	delete myVulkanEngine;
	delete myVulkanScene;
}

void App::Run()
{
	WonderfulEditor editor;
	editor.Init();
	
	while (!glfwWindowShouldClose(myWindow))
	{
		glfwPollEvents();
		KeyboardInput::UpdateInput();
		MouseInput::UpdateMouseInput();

		myVulkanEngine->BeginImgui();
		myVulkanEngine->BeginRender();
		
		editor.Update();
		
		Vulkan::DisplayDebugTap();
		
		myVulkanEngine->RenderImGui();
		myVulkanEngine->EndRender();
	}
	
	editor.Destroy();
}

void App::BuildGLFWWindow(int aWidth, int aHeight)
{
	//initialize glfw
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//GLFWwindow* glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
	myWindow = glfwCreateWindow(aWidth, aHeight, "ID Tech 12", nullptr, nullptr);
	if (myWindow != nullptr) {
		LOG_INFO("Successfully made a glfw window called \"ID Tech 12\", width: %d, height: %d", aWidth, aHeight);
	}
	else {
		LOG_ERROR("%s","Failed to create a glfw window");
	}
}
