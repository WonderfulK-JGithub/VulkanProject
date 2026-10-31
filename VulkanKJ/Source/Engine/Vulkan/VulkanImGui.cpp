#include "VulkanImGui.h"
//#include <vulkan/vulkan.h>

#ifndef DONT_USE_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#endif // !DONT_USE_IMGUI

#include <imgui_internal.h>
#include <IconFontHeaders/IconsLucide.h>

#include "VkInit/Descriptors.h"
#include "Engine.h"
#include "VkInit/Instance.h"
#include "VkInit/VkCommands.h"
#include "SingleTimeCommands.h"
#include "VkInit/RenderPass.h"

static void CheckVKResult(VkResult err)
{
	if (err == 0) {
		return;
	}

	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

	if (err < 0) {
		abort();
	}
}

static ImFont* ImGuiLoadSystemFont(ImFontAtlas& atlas, const char* name, float size)
{
	char* windir = nullptr;
	if (_dupenv_s(&windir, nullptr, "WINDIR") || windir == nullptr)
	{
		return nullptr;
	}

	static const ImWchar ranges[] = {
		0x0020,
		0x00FF,
		// Basic Latin + Latin Supplement
		0x0104,
		0x017C,
		// Polish characters and more
		0,
	};

	ImFontConfig config;
	config.OversampleH = 4;
	config.OversampleV = 4;
	config.PixelSnapH  = false;

	std::string path = std::string(windir) + "\\Fonts\\" + name;
	ImFont* font     = atlas.AddFontFromFileTTF(path.c_str(), size, &config, ranges);

	free(windir);

	return font;
}

void ImGUIRenderer::Init(GLFWwindow* aWindow, Engine& aEngine)
{
	myEngine = &aEngine;
	myWindow = aWindow;

	CreateDescriptorPool();

	InitImGUI();
}

void ImGUIRenderer::Destroy()
{
	
#ifndef DONT_USE_IMGUI
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif
	
	myEngine->GetLogicalDevice().destroyDescriptorPool(m_descriptorPool);
}

void ImGUIRenderer::BeginFrame()
{
#ifndef DONT_USE_IMGUI
	ImFontAtlasUpdateNewFrame(myFontAtlas, ImGui::GetFrameCount(), true);
	
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
#endif
}

void ImGUIRenderer::EndFrame([[maybe_unused]]vk::CommandBuffer aCommandBuffer)
{
#ifndef DONT_USE_IMGUI
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), aCommandBuffer);

	
#endif
}

void ImGUIRenderer::Present()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ImGUIRenderer::CreateDescriptorPool()
{

	VkInit::DescriptorSetLayoutData input;
	
	input.types.push_back(vk::DescriptorType::eSampler);
	input.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	input.types.push_back(vk::DescriptorType::eSampledImage);
	input.types.push_back(vk::DescriptorType::eStorageImage);
	input.types.push_back(vk::DescriptorType::eUniformTexelBuffer);
	input.types.push_back(vk::DescriptorType::eStorageTexelBuffer);
	input.types.push_back(vk::DescriptorType::eUniformBuffer);
	input.types.push_back(vk::DescriptorType::eStorageBuffer);

	input.types.push_back(vk::DescriptorType::eUniformBufferDynamic);
	input.types.push_back(vk::DescriptorType::eStorageBufferDynamic);
	input.types.push_back(vk::DescriptorType::eInputAttachment);
	input.count      = static_cast<int>(input.types.size());
	input.flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	m_descriptorPool = VkInit::MakeDescriptorPool(myEngine->GetLogicalDevice(), 1000, input);
}

void ImGUIRenderer::InitImGUI()
{
#ifndef DONT_USE_IMGUI
	{
		myFontAtlas = IM_NEW(ImFontAtlas);
		myFonts.defaultFont = ImGuiLoadSystemFont(*myFontAtlas, "segoeui.ttf", 18.0f); //16.0f * 96.0f / 72.0f);

		{
			float iconFontSize = 18.0f;

			//static constexpr ImWchar icons_ranges[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};
			ImFontConfig icons_config;
			icons_config.MergeMode        = true;
			icons_config.OversampleH      = 4;
			icons_config.OversampleV      = 4;
			icons_config.PixelSnapH       = true;
			icons_config.GlyphMinAdvanceX = iconFontSize;
			icons_config.GlyphOffset      = {0.f, 3.f};
			std::string fontPath          = (Wonder::GetEngineAssetPath() / "Fonts/lucide.ttf").string();

			myFonts.iconFont = myFontAtlas->AddFontFromFileTTF(fontPath.c_str(), iconFontSize, &icons_config);
		}

		myFonts.defaultFontLarge = ImGuiLoadSystemFont(*myFontAtlas, "segoeui.ttf", 24.0f); //16.0f * 96.0f / 72.0f);

		// todo: load variations like bold and at different sizes, expose through ImGuiInterface;

		{
			float iconFontSize = 24.0f;

			static constexpr ImWchar icons_ranges[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};
			ImFontConfig icons_config;
			icons_config.MergeMode        = true;
			icons_config.OversampleH      = 4;
			icons_config.OversampleV      = 4;
			icons_config.PixelSnapH       = true;
			icons_config.GlyphMinAdvanceX = iconFontSize;
			icons_config.GlyphOffset      = {0.f, 3.f};
			std::string fontPath          = (Wonder::GetEngineAssetPath() / "Fonts/lucide.ttf").string();
			myFonts.iconFontLarge        = myFontAtlas->AddFontFromFileTTF(fontPath.c_str(), iconFontSize, &icons_config, icons_ranges);
		}

		{
			float iconFontSize = 8.0f;

			//static constexpr ImWchar icons_ranges[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};
			ImFontConfig icons_config;
			icons_config.OversampleH      = 1;
			icons_config.OversampleV      = 1;
			icons_config.PixelSnapH       = true;
			icons_config.GlyphMinAdvanceX = iconFontSize;
			icons_config.GlyphOffset      = {0.f, 0.f};
			std::string fontPath          = (Wonder::GetEngineAssetPath() / "Fonts/lucide.ttf").string();
			
			myFonts.iconFontSmall        = myFontAtlas->AddFontFromFileTTF(fontPath.c_str(), iconFontSize, &icons_config);
		}

		myFontAtlas->RendererHasTextures = true;
		myFontAtlas->Build();
		
	}
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(myFontAtlas);

	ImGuiIO& io      = ImGui::GetIO();
	io.ConfigFlags   |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags   |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.ConfigFlags   |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags   |= ImGuiConfigFlags_ViewportsEnable;
	io.DisplaySize.x = static_cast<float>(myEngine->GetSwapchainExtent().width);
	io.DisplaySize.y = static_cast<float>(myEngine->GetSwapchainExtent().height);

	ImGui::GetStyle().FontScaleMain = 1.f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	bool InstallGLFWCallbacks = false;
	ImGui_ImplGlfw_InitForVulkan(myWindow, InstallGLFWCallbacks);

	ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
	pipelineInfo.RenderPass = VkInit::GetRenderPass(Vulkan::RenderStageType::ImGui);
	pipelineInfo.Subpass = 0;


	ImGui_ImplVulkan_InitInfo InitInfo = {
		.ApiVersion = VkInit::GetInstanceVersion(),
		.Instance = myEngine->GetVulkanInstance(),
		.PhysicalDevice = myEngine->GetPhysicalDevice(),
		.Device = myEngine->GetLogicalDevice(),
		.QueueFamily = myEngine->GetQueueFamilyIndices().graphicsFamily.value(),
		.Queue = myEngine->GetGraphicsQueue(),
		.DescriptorPool = m_descriptorPool,
		.MinImageCount = myEngine->GetSwapchainMinImages(),
		.ImageCount = static_cast<uint32_t>(myEngine->GetSwapchainFrames().size()),
		.PipelineInfoMain = pipelineInfo,
		.CheckVkResultFn = CheckVKResult,
		
	};

	InitInfo.UseDynamicRendering = false;

	ImGui_ImplVulkan_Init(&InitInfo);

#endif

}
