#include "VulkanApplication.hpp"



int main() {
	VulkanApplication& app = VulkanApplication::getInstance();

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << "CAUGHT ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
