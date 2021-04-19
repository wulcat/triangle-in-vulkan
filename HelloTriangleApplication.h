#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <optional> // c++17 introduced a data structure to distinguish betwen the case of a value exisiting or not
#include <set> // elements are unique and cannot be modified once added but add/remove can be done
#include <cstdint> // necessary for uint32_max (do reserach on this)
#include <algorithm> // do reserach on this as well

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// std::vector is sequence container
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// for swap-chains
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilties;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication {
public:
	void run();
private:
	GLFWwindow* window; // pointer variable which stores memory address of variable

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface; // device and surface are core components of swap chain

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue; // for window surface (presentational queue)

	VkSwapchainKHR swapChain;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	void initWindow(); // setup the glfw for application
	void initVulkan(); // setup the vulkan
	void setupDebugMessenger();
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources

	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createImageViews();

	void getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount);
	void getAndPrintSupportedExtensions();

	bool checkValidationLayerSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device); // checks for best and support gpu (usefull for VR to check support for 64 bit floats and multi viewport rendering)

	std::vector<const char*> getRequiredExtensions();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device); // finds graphics queue family
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	// do some research on this function
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void pickPhysicalDevice();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilties); // resolution of swap chain images (almost equal to resolution of window)
	
};

