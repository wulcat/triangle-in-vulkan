#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication {
public:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	void run();
private:
	GLFWwindow* window;

	void initWindow();
	void initVulkan(); // setup the vulkan
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources
};

