#include "manager.hpp"

#include "graphics.hpp"
#include "renderer.hpp"
#include "descriptor.hpp"
#include "utilities.hpp"
#include "input.hpp"
#include "time.hpp"
#include "pipeline.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "pass.hpp"
#include "bitmask.hpp"
#include "command.hpp"
#include "vertex.hpp"
#include "shape.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

struct UniformData
{
	dpoint2D center;
	double zoom;
	float aspect;
	int maxIterations;
};

void Test(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	static bool start = true;
	static Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT16> mesh;
	static Pass pass;
	static Pipeline pipeline;
	static UniformData uniformData;
	static Buffer buffer;
	static Descriptor descriptor;

	if (start)
	{
		uniformData.center = dpoint2D({-0.5, 0.0});
		uniformData.zoom = 1.0;
		uniformData.aspect = float(Manager::GetWindow().GetConfig().extent.width) / float(Manager::GetWindow().GetConfig().extent.height);
		uniformData.maxIterations = 30;

		mesh.SetShape(Shape<Position | Coordinate, VK_INDEX_TYPE_UINT16>(ShapeType::Quad));

		mesh.Create(&Manager::GetDevice());

		PassConfig passConfig = Pass::DefaultConfig();
		pass.Create(passConfig, &Manager::GetDevice());

		BufferConfig bufferConfig{};
		bufferConfig.mapped = true;
		bufferConfig.size = sizeof(uniformData);
		buffer.Create(bufferConfig, &Manager::GetDevice(), &uniformData);

		std::vector<DescriptorConfig> descriptorConfigs(1);
		descriptorConfigs[0].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor.Create(descriptorConfigs, &Manager::GetDevice());
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer.GetBuffer();
		bufferInfo.range = sizeof(uniformData);
		descriptor.Update(0, &bufferInfo, nullptr);

		PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
		pipelineConfig.shader = "screen";
		pipelineConfig.vertexInfo = mesh.GetVertexInfo();
		pipelineConfig.renderpass = pass.GetRenderpass();
		pipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
		pipelineConfig.descriptorLayouts = {descriptor.GetLayout()};
		pipeline.Create(pipelineConfig, &Manager::GetDevice());

		Renderer::AddPass(&pass);

		start = false;
	}
	else if (commandBuffer)
	{
		if (Input::GetKey(GLFW_KEY_W).down) uniformData.center.y() -= Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_S).down) uniformData.center.y() += Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_A).down) uniformData.center.x() -= Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_D).down) uniformData.center.x() += Time::deltaTime / uniformData.zoom;

		if (Input::GetKey(GLFW_KEY_UP).down) uniformData.zoom += Time::deltaTime * uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_DOWN).down) uniformData.zoom -= Time::deltaTime * uniformData.zoom;

		if (Input::GetKey(GLFW_KEY_RIGHT).pressed || (Input::GetKey(GLFW_KEY_RIGHT).holding && Time::newTick)) 
			uniformData.maxIterations += 2;
		if (Input::GetKey(GLFW_KEY_LEFT).pressed || (Input::GetKey(GLFW_KEY_LEFT).holding && Time::newTick)) 
			uniformData.maxIterations -= 2;

		buffer.Update(&uniformData, sizeof(uniformData));

		pipeline.Bind(commandBuffer);
		descriptor.Bind(commandBuffer, pipeline.GetLayout());
		mesh.Bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, CUI(mesh.GetIndices().size()), 1, 0, 0, 0);
	}
	else
	{
		mesh.Destroy();
		buffer.Destroy();
		descriptor.Destroy();
		pass.Destroy();
		pipeline.Destroy();

		start = true;
	}
}

void Start()
{
	Test(nullptr, 0);
	Renderer::RegisterCall(Test);
}

void End()
{
	Test(nullptr, 0);
}

int main(int argc, char** argv)
{
	Manager::ParseArguments(argv, argc);
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Manager::RegisterEndCall(End);

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}