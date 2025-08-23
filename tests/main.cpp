#include "context/VulkanApplication.hpp"


const std::string FIRST_PASS_VERT_SHADER_PATH = "../../VulkanProject/assets/shaders/vert.spv";
const std::string FIRST_PASS_FRAG_SHADER_PATH = "../../VulkanProject/assets/shaders/frag.spv";

const std::string SECOND_PASS_VERT_SHADER_PATH = "../../VulkanProject/assets/shaders/secondPassVert.spv";
const std::string SECOND_PASS_FRAG_SHADER_PATH = "../../VulkanProject/assets/shaders/secondPassFrag.spv";

const std::string MODEL_PATH = "../../VulkanProject/assets/models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "../../VulkanProject/assets/models/viking_room/viking_room.png";
const std::string TEXTURE2_PATH = "../../VulkanProject/assets/models/viking_room/normal_texture_test.png";


void createWorldObjects(VulkanAppParams& params, VulkanApplication& app) {

	Scene& scene = *params.scene;

	// LOAD MESHES
	Mesh* mesh = app.createMesh(MODEL_PATH);
	// LOAD TEXTURES
	Material* material = app.createMaterial({TEXTURE_PATH, TEXTURE2_PATH});

	// MODEL
	Entity* modelEntity = scene.addEntity();
	Model* model = modelEntity->addModule<Model>();
	model->create(mesh, material);

	// LIGHTS
	Entity* lightTest = scene.addEntity();
	Entity* lightTest2 = scene.addEntity();
	Light* lightM1 = lightTest->addModule<Light>();
	Light* lightM2 = lightTest2->addModule<Light>();
	lightM2->setColor(glm::vec3(1.0f, 0.0f, 0.0f));

	Entity* cameraEntity = scene.addEntity();
	cameraEntity->transform.lookAt = -Transform::Y;
	cameraEntity->transform.right = -Transform::X;
	cameraEntity->transform.position.y = 5;
	cameraEntity->transform.position.z = 0.5;
	Camera* camera = cameraEntity->addModule<Camera>();
	scene.activeCamera = camera;
}

int main() {
	Scene scene;

	VulkanAppParams params = {};
	params.applicationName = "VULKAN TEST";
	params.scene = &scene;

	params.firstRenderPassVertShaderPath = FIRST_PASS_VERT_SHADER_PATH;
	params.firstRenderPassFragShaderPath = FIRST_PASS_FRAG_SHADER_PATH;
	params.secondRenderPassVertShaderPath = SECOND_PASS_VERT_SHADER_PATH;
	params.secondRenderPassFragShaderPath = SECOND_PASS_FRAG_SHADER_PATH;

	try {
		VulkanApplication app(params);
		createWorldObjects(params, app);
		app.run(params);
	}
	catch (const std::exception& e) {
		std::cerr << "CAUGHT ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
