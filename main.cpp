#include<vulkan/vulkan.h> // provides functions, structures and enumerations from LunarG SDK

#include <iostream> // for logging/reporting
#include <stdexcept> // for propogating errors
#include <cstdlib> // Provides macros like exit success and failure

#include "HelloTriangleApplication.h"

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}