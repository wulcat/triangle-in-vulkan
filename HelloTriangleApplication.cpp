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
}

void HelloTriangleApplication::mainLoop() {
	// Add event loop until the application is closed (this look also basically checks for event inputs)
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
	}
}


// After the application is closed destroy the objects
void HelloTriangleApplication::cleanup() {
	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);

	glfwTerminate();
}

void HelloTriangleApplication::createInstance() {
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

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	getAndPrintRequiredExtensions(glfwExtensions, glfwExtensionCount);
	getAndPrintSupportedExtensions();

	// Create Vulkan Instance here
	//VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}


void HelloTriangleApplication::getAndPrintRequiredExtensions(const char** glfwExtensions, uint32_t glfwExtensionCount) {
	std::cout << "Required Extensions: \n";

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
