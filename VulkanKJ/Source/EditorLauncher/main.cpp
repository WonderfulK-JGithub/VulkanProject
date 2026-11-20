#include "App.h"

int main() {
	_putenv_s(
	"VK_DRIVER_FILES",
	R"(C:\WINDOWS\System32\DriverStore\FileRepository\nvhdci.inf_amd64_f92832a3cbda33f7\nv-vk64.json)"
);
	
	_putenv_s("VK_LOADER_LAYERS_DISABLE", "~all~");
	_putenv_s("VK_LOADER_LAYERS_ENABLE", "VK_LAYER_KHRONOS_validation");
	_putenv_s("VK_LOADER_DEBUG", "error,warn,driver");
	App* app = new App(1280,720);

	app->Run();

	delete app;

	return 0;
}