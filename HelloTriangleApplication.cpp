#include "HelloTriangleApplication.h"

void HelloTriangleApplication::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void HelloTriangleApplication::initWindow() {
	// Init GLFW library
	glfwInit();

	// Say no to init for open gl & turn of auto reize for now
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create popup window with given w,d and title (4th param represents monitor and 5th is for opengl)
	this->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {
	createInstance();
	setupDebugMessenger();
	pickPhysicalDevice();
}

void HelloTriangleApplication::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	// above struct a vkCreateDebugUtilMessengerEXT function to create an object.
	// Since the function is an extension function, it's not loaded automatically.
	// So we create it by looking at the address using vkGetInstanceProcAddr.
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger");
	}
}

void HelloTriangleApplication::mainLoop() {
	// Add event loop until the application is closed (this look also basically checks for event inputs)
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
	}
}


// After the application is closed destroy the objects
void HelloTriangleApplication::cleanup() {
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);

	glfwTerminate();
}

void HelloTriangleApplication::createInstance() {
	// if debug is on and no validation layers are found for error verbose and profiling throw error
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}


	// Create struct and pass it to vulkan to create instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get the required extensions from glfw
	auto extensions = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	 // Print about extensions (remove as its not really necessary)
	 getAndPrintRequiredExtensions(extensions.data(), extensions.size());
	 getAndPrintSupportedExtensions();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// Set the validation layers for debugging, profiling and tracing
	if (enableValidationLayers) {
		// for validation layer
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// for debug messenger
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		// for debug messenger
		createInfo.pNext = nullptr;
	}

	// Create Vulkan Instance here
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void HelloTriangleApplication::pickPhysicalDevice() {
	// We find the available gpu count (if not we end the application)
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with vulkan support");
	}

	// We allocate an array to hold all of available physical devices (gpu)
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Check if the physical device meets the requirment for running this application
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find suitable physical device (GPU)");
	}


}


void HelloTriangleApplication::getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount) {
	std::cout << "Required Extensions: \n";

	// warning over here: warning C4018: '<': signed/unsigned mismatch
	for (int i = 0; i < glfwExtensionCount; i++) {
		printf("\t%s\n", glfwExtensions[i]);
	}
}

void HelloTriangleApplication::getAndPrintSupportedExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "Available extensions\n";
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}
}

bool HelloTriangleApplication::checkValidationLayerSupport() {
	uint32_t layerCount;
	// pass the memory address of layercount (&layerCount)
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr); 
	
	// vecotr is sequence container like array
	std::vector<VkLayerProperties> availableLayers(layerCount);

	// vector.data() returns direct pointer to memory array used internally by vector to store its owned elements
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// this is range - for loop
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		// const type& name : vector - get a reference from the sequnce container
		for (const VkLayerProperties& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName)) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0; // unsigned fixed 32 bit integer
	const char** glfwExtensions; // pointer to pointer of char (like 2D array or array of arrays of char)
	
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// what is this statement :/
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		// VK_EXT_debug_utils is same macro as VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device) {
	//VkPhysicalDeviceProperties deviceProperties;
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	//// instead of checking if a device is suitable or not, we can pick the best device with higher score
	//// check the tutorial for this example (page 86: Base device sutablity checks)
	//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	QueueFamilyIndices indices = findQueueFamilies(device);

	return indices.isComplete();
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// logic to find queue family indices to populate struct with
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	
	// find atleast one queue family which supports VK_QUEUE_GRAPHICS_BITS
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // revised: `&` find's the common bit 
			indices.graphicsFamily = i;
		}

		// for early exit since we have foudn suitable queue
		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}


VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
//	return VkResult();
//}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
