#include "VulkanApplication.hpp"


const std::string FIRST_PASS_VERT_SHADER_PATH = "../../VulkanProject/assets/shaders/vert.spv";
const std::string FIRST_PASS_FRAG_SHADER_PATH = "../../VulkanProject/assets/shaders/frag.spv";

const std::string SECOND_PASS_VERT_SHADER_PATH = "../../VulkanProject/assets/shaders/secondPassVert.spv";
const std::string SECOND_PASS_FRAG_SHADER_PATH = "../../VulkanProject/assets/shaders/secondPassFrag.spv";

const std::string MODEL_PATH = "../../VulkanProject/assets/models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "../../VulkanProject/assets/models/viking_room/viking_room.png";


int main() {
	VulkanApplication& app = VulkanApplication::getInstance();
	VulkanAppParams params = {};
	params.firstRenderPassVertShaderPath = FIRST_PASS_VERT_SHADER_PATH;
	params.firstRenderPassFragShaderPath = FIRST_PASS_FRAG_SHADER_PATH;
	params.secondRenderPassVertShaderPath = SECOND_PASS_VERT_SHADER_PATH;
	params.secondRenderPassFragShaderPath = SECOND_PASS_FRAG_SHADER_PATH;
	params.modelPath = MODEL_PATH;
	params.texturePath = TEXTURE_PATH;

	try {
		app.run(params);
	}
	catch (const std::exception& e) {
		std::cerr << "CAUGHT ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
