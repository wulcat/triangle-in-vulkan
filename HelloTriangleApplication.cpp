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
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
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
		drawFrame();
	}

	// wait for device to finish up all the queue commands before we go ahead and cleaup memory
	vkDeviceWaitIdle(device);
}


// After the application is closed destroy the objects
void HelloTriangleApplication::cleanup() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(this->instance, nullptr);

	glfwDestroyWindow(this->window);

	glfwTerminate();
}

void HelloTriangleApplication::drawFrame() {
	// all of these is executed async :() so!
	// the returns from the swap chain events are unordered so we use fences or semaphores to sync
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// acquire the image from the swap chain
	uint32_t imageIndex; // index for vkimage from swapchainimages
	// logical device :,: swap chain for image to acqure from, 
	// timeout in nanoseconds(using max 64bit uint disables the timeout)
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	// Check if previous frame is using this frame
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Mark the image as now being use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	// execute the command buffer with that image as attachment in the frame buffer

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer");
	}

	// return the image to the swap for presentation
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;
	// presentInfo.pResults = nullptr; // optinal

	vkQueuePresentKHR(presentQueue, &presentInfo);
	vkQueueWaitIdle(presentQueue); // make cpu wait until gpu is free

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // if i pass nullptr instead of devices.data(), the class variable devices returns null

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

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// if the above fails we just use the first available format
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // triple buffering low latency no tearing
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; // similar to vsync (double buffering) no tearing
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilties) {
	if (capabilties.currentExtent.width != UINT32_MAX) {
		return capabilties.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

		// clamp the WIDTH and HEIGHT to availalbe min and max supported by our implementation
		actualExtent.width = std::max(capabilties.minImageExtent.width, std::min(capabilties.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilties.minImageExtent.height, std::min(capabilties.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

// shader modules are thin wrappers around teh shader bytecode (spir-v)
VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // check what is this line

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}

void HelloTriangleApplication::createSurface() {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}

void HelloTriangleApplication::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilties);

	// to avoid waiting before internal operations are complete we request one additional image
	uint32_t imageCount = swapChainSupport.capabilties.minImageCount + 1;

	if (swapChainSupport.capabilties.maxImageCount > 0 && imageCount > swapChainSupport.capabilties.maxImageCount) {
		imageCount = swapChainSupport.capabilties.maxImageCount;
	}

	// as a tradtion with vulkan objects, creating the swap chain object requires filling in a large structure
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent; // safe comment-out (validation layer detects this mistake)
	createInfo.imageArrayLayers = 1; // amount layers each image consists of (only useful for 3d stereoscopic app)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // for post-processing use vk_image_usage_transfer_dst_bit

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // optional
		createInfo.pQueueFamilyIndices = nullptr; // optional
	}

	// soemthign realated to rotation (90 degree clockwise or horizontal flip)
	createInfo.preTransform = swapChainSupport.capabilties.currentTransform;
	// we want to ignore the alpha channel
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// clip the pixels which are not presenting (visible on window)
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	// in vulkan swap chain can become invalid or unoptimized while running application
	// if we resize the window the swap chain has to be recreated from scratch (todo in later chapters)
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}


	// Add handles
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount); // set the size with max image count availabe from swap chain
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		// the following parameter allows us to treat a image 1d / 2d / 3d texture
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		//  allows us to swizzle the color channels (for ex it cna let us create monochrome texture)
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// describles the purpose of image and which part of image should be accessed
		// here our iamge will be used as color targets wihout mipmapping levels or mulitple layers
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view: " + i);
		}
	}
}

void HelloTriangleApplication::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // since we are not doing multisampling stick to 1bit

	// determin what to do before and after rendering
	// we are only interested to see the triangle so we will store the triangle data
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	
	// since our application has no stencil buffer, storing and loading is irrelevant
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// images need to be tranistioned to specific layhouts that are suitable for the operation that they are going to be involved in next
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// subpassing and atatchment references
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	//
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}

void HelloTriangleApplication::createGraphicsPipeline() {
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // pipeline stage this shader is going into

	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	// look into pSpecializatinInfo struct 

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// Fixed Function Implementation

	// binding = spacing between data and whetehr the data is per-vertex or per-instance
	// attribute descriptions: ????/

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // optinal

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewports define the transformation from the image to the framebuffer
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// scissor rectangles define in which regions pixls will be actaully stored (any pixel outside scissor will be discarded by rasterizer)
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	// to use multile viewports and scissor rectanglers need to 
	// enabling multiple gpu can be done from logical device creation
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// rasterizer
	// takes a geomtery from vertex shader and turns it into fragments to be colored by the fragment shader
	// also performs depth testing, face culling and the scissor test and it can be configured to output fragments that fill enteire polygons
	// or just the edgse (wireframe rendering)
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // making this true requires gpu features (usefull for shadow maps)
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // if true the geomtery never passes through rasterizer stage. (basically disables output frame buffer)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // can be used to showcase different view modes (fill, gemotery - requires - gpu, vertex - requires gpu)
	rasterizer.lineWidth = 1.0f; // describles the width of line in terms of number of fragments. max depends upon the gpu and more 1.0f requires wideLines gpu feature
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE; // mostly used for shadow mapping
	rasterizer.depthBiasConstantFactor = 0.0f; // optional
	rasterizer.depthBiasClamp = 0.0f; // optinal
	rasterizer.depthBiasSlopeFactor = 0.0f; // optinal
	
	// multipsampling - anti-aliasing - enabling requres gpu featuere
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // optional
	multisampling.pSampleMask = nullptr; // optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // optional
	multisampling.alphaToOneEnable = VK_FALSE; // optional

	// depth and stnecil testing - we'll get back to it in the depth buffering chapter
	// VKpi\peline dpeth stencil state create info

	// color blending - the color returned by fragment shader needs to be combined with the color that is already in the framebuffer
	// mix the old and new to produce final
	// combine the old and new using bitwase operation
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // optinal
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // optinal
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // optinal
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // optinal 
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // optinal
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // optinal
	
	// the above config allows the first way to color blend

	/*if (blendEnable) { // psuedo code 
		finalColor.rgb = (srcColorBlendFactor * newColor.rgb) < colorBlendOp > (dstColorBlendFactor * oldColor.rgb);
		finalColor.a = (srcAlphaBlendFactor * newColor.a) < alphaBlendOp > (dstAlphaBlendFactor * oldColor.a);
	}
	else {
		finalColor = newColor;
	}

	finalColor = finalColor & colorWriteMask;*/

	// as for we disablled both modes to it will be written to the framebuffer unmodfied
	// this structure references the array of structures for all of the framebuffers and allow us to set blend constants
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // if we use this as true this will disable the first color blending and we have to use the second blending method
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // optional
	colorBlending.blendConstants[1] = 0.0f; // optional
	colorBlending.blendConstants[2] = 0.0f; // optional
	colorBlending.blendConstants[3] = 0.0f; // optional

	//// dyanamic state - come back to this later for dyanmic changes (enabling this will ignore the config values for these)
	//VkDynamicState dynamicStates[] = {
	//	VK_DYNAMIC_STATE_VIEWPORT,
	//	VK_DYNAMIC_STATE_LINE_WIDTH
	//};

	//VkPipelineDynamicStateCreateInfo dynamicState{};
	//dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	//dynamicState.pDynamicStates = dynamicStates;
		
	// pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
		throw std::runtime_error("failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	// fixed function stage references
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	// vulkan handle rather than struc pointer
	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // optional
	pipelineInfo.basePipelineIndex = -1; // optional


	// second param is related pipeline cache for reuse purpose f
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}

	// cleanup right away after use
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void HelloTriangleApplication::createFrameBuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) { // unsigned int: used for size or count in bytes
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void HelloTriangleApplication::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0; // optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}


void HelloTriangleApplication::createCommandBuffers() {
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate commnad buffers!");
	}

	// recording command buffers
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // optional
		beginInfo.pInheritanceInfo = nullptr; // optional (only relevant for second commnad buffer)

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		// starting the render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		// we created a framebuffer for each swap chain image that specifies it as color attachment
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i]; 

		// size of render area
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// The base color after clearing the frame 
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// command buffer, vertexCount (3 vertices of traingle), 1 instanceCount for rendering, defines lowest value of gl_VertextIndex, defines lowes value of gl_InstanceIndex
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0); 

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}




void HelloTriangleApplication::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sync objects in a frame!");
		}
	}
}

void HelloTriangleApplication::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	// vulkan lets us assign priorites to queues to influce the scheduling of command buffer execution
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Adding queue family data
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	
	// Enabling device features with no extensions
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// compared to createInstance of vulkan this requies to specify extensions and validation layers
	// this time this is device specific
	// an eg. is VK_KHR_swapchain - allows you to render present images from a device to windows
	if (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	// lets create the logical device
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to craete logical device");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}


void HelloTriangleApplication::getAndPrintRequiredExtensions(const char** glfwExtensions, size_t glfwExtensionCount) {
	std::cout << "Required Extensions: \n";

	// warning over here: warning C4018: '<': signed/unsigned mismatch
	for (size_t i = 0; i < glfwExtensionCount; i++) {
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

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName); // removes and shrinks the set size
	}

	return requiredExtensions.empty(); // returns wheather the set empty or not (bool)
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

	// important we try to only query for swap chain support after verifying that the extension is available
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;	
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// logic to find queue family indices to populate struct with
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); // error (device is null)
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());	
	
	// find atleast one queue family which supports VK_QUEUE_GRAPHICS_BITS
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // revised: `&` find's the common bit 
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		// for early exit since we have foudn suitable queue
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilties);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount; 
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

std::vector<char> HelloTriangleApplication::readFile(const std::string& fileName) {
	// ate: start reading at the end of file (advantage is we can determinze the size of file and allocate a buffer)*
	// binary: read the file as binary file (avoid text transformations)
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file " + fileName);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
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

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
