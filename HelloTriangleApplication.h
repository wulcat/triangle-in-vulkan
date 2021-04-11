#pragma once

class HelloTriangleApplication {
public:
	void run();
private:
	void initVulkan(); // setup the vulkan
	void mainLoop(); // to render on window
	void cleanup(); // after closing window, deallocating resources
};

