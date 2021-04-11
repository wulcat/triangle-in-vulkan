#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
public:
	void run();
private:
	GLFWwindow* window;
	VkInstance instance;

	void initWindow();
	void initVulkan(); // setup the vulkan
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources

	void createInstance();

	void getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount);
	void getAndPrintSupportedExtensions();
};

