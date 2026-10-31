#include "App.h"

int main() {
	App* app = new App(1280,720);

	app->Run();

	delete app;

	return 0;
}