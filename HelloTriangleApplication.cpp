#include "HelloTriangleApplication.h"

void HelloTriangleApplication::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void HelloTriangleApplication::initWindow() {
	// Init GLFW (auto init's vulkan sdk)
	glfwInit();

	// Say no to init for open gl & turn of auto reize for now
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create popup window with given w,d and title (4th param represents monitor and 5th is for opengl)
	this->window = glfwCreateWindow(this->WIDTH, this->HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {
}

void HelloTriangleApplication::mainLoop() {
	// Add event loop until the application is closed (this look also basically checks for event inputs)
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
	}
}


// After the application is closed destroy the objects
void HelloTriangleApplication::cleanup() {
	glfwDestroyWindow(this->window);

	glfwTerminate();
}
