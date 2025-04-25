#include "Mesh.hpp"

#include "asset/modelLoader.hpp"


void destroyMesh(Device device, Mesh& mesh) {
	vkDestroyBuffer(device.get(), mesh.vertexBuffer, nullptr);
	mesh.vertexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(device.get(), mesh.vertexBufferMemory, nullptr);
	mesh.vertexBufferMemory = VK_NULL_HANDLE;
	vkDestroyBuffer(device.get(), mesh.indexBuffer, nullptr);
	mesh.indexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(device.get(), mesh.indexBufferMemory, nullptr);
	mesh.indexBufferMemory = VK_NULL_HANDLE;
}
