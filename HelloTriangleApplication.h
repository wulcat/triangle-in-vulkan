#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// std::vector is sequence container
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


class HelloTriangleApplication {
public:
	void run();
private:
	GLFWwindow* window; // pointer variable which stores memory address of variable
	VkInstance instance;

	void initWindow(); // setup the glfw for application
	void initVulkan(); // setup the vulkan
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources

	void createInstance();

	void getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount);
	void getAndPrintSupportedExtensions();

	bool checkValidationLayerSupport();
};

