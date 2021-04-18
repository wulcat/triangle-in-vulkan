#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <optional> // c++17 introduced a data structure to distinguish betwen the case of a value exisiting or not

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

// according to documentaion this is a proxy function (basically proxy pattern)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices {
	// optional has no value by default until a value has assigned
	// graphicsFamily.has_value() will return false until a value has been assigned
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily; // for checking if window surface supports the drawiing

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class HelloTriangleApplication {
public:
	void run();
private:
	GLFWwindow* window; // pointer variable which stores memory address of variable

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;

	void initWindow(); // setup the glfw for application
	void initVulkan(); // setup the vulkan
	void setupDebugMessenger();
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources

	void createInstance();
	void createLogicalDevice();

	void getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount);
	void getAndPrintSupportedExtensions();

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device); // checks for best and support gpu (usefull for VR to check support for 64 bit floats and multi viewport rendering)
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device); // finds graphics queue family

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void pickPhysicalDevice();

	void createSurface();
};

