#include "Loden/GUI/AgpuCanvas.hpp"

namespace Loden
{
namespace GUI
{

AgpuCanvas::AgpuCanvas()
{
	vertexCapacity = 0;
	indexCapacity = 0;
}

AgpuCanvas::~AgpuCanvas()
{
}

AgpuCanvasPtr AgpuCanvas::create(const PipelineStateManagerPtr &stateManager)
{
	auto &device = stateManager->getDevice();

	auto layout = stateManager->getVertexLayout("CanvasVertex2D");
	if(!layout)
		return nullptr;

	// Create the command list allocator.
	auto allocator = device->createCommandAllocator(AGPU_COMMAND_LIST_TYPE_BUNDLE);
	if(!allocator)
		return nullptr;

	// Create the command list.
	auto commandList = device->createCommandList(AGPU_COMMAND_LIST_TYPE_BUNDLE, allocator, nullptr);
	if(!commandList)
		return nullptr;
	commandList->close();

	// Create the canvas object.
	auto canvas = AgpuCanvasPtr(new AgpuCanvas());
	canvas->stateManager = stateManager;
	canvas->device = device;
	canvas->allocator = allocator;
	canvas->commandList = commandList;
	canvas->vertexBufferBinding = device->createVertexBinding(layout.get());
    canvas->shaderSignature = stateManager->getShaderSignature("GUI");
	canvas->linePipeline = stateManager->getPipelineState("canvas2d.color.line");
	canvas->trianglePipeline = stateManager->getPipelineState("canvas2d.color.triangle");

	return canvas;
}

void AgpuCanvas::createVertexBuffer()
{
	vertexCapacity = vertices.size();

    agpu_buffer_description desc;
    desc.size = agpu_uint(vertexCapacity*sizeof(Vertex));
    desc.usage = AGPU_STREAM;
    desc.binding = AGPU_ARRAY_BUFFER;
    desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
    desc.stride = agpu_uint(sizeof(Vertex));
    vertexBuffer = device->createBuffer(&desc, nullptr);

	// Update the vertex buffer binding
	vertexBufferBinding->bindVertexBuffers(1, (agpu_buffer**)&vertexBuffer);
}

void AgpuCanvas::createIndexBuffer()
{
	indexCapacity = indices.size();

    agpu_buffer_description desc;
    desc.size = agpu_uint(indexCapacity*sizeof(int));
    desc.usage = AGPU_STREAM;
    desc.binding = AGPU_ELEMENT_ARRAY_BUFFER;
    desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
    desc.stride = agpu_uint(sizeof(int));
    indexBuffer = device->createBuffer(&desc, nullptr);
}

void AgpuCanvas::reset()
{
	baseVertex = 0;
	startIndex = 0;
	shapeType = ST_Unknown;
	drawCommandsToAdd.clear();

	shapeType = ST_Unknown;
	vertices.clear();
	indices.clear();
	allocator->reset();
	commandList->reset(allocator.get(), nullptr);
}

void AgpuCanvas::close()
{
	if(vertices.empty() || indices.empty())
	{
		commandList->close();
		return;
	}
	endSubmesh();

	if(vertexCapacity < vertices.size())
		createVertexBuffer();
	if(indexCapacity < indices.size())
		createIndexBuffer();
	vertexBuffer->uploadBufferData(0, vertices.size()*sizeof(Vertex), &vertices[0]);
	indexBuffer->uploadBufferData(0, indices.size()*sizeof(int), &indices[0]);

    // Set the shader signature.
    commandList->setShaderSignature(shaderSignature.get());

	// Store the commands in the command list.
	commandList->useVertexBinding(vertexBufferBinding.get());
	commandList->useIndexBuffer(indexBuffer.get());
	for(auto &command : drawCommandsToAdd)
		command();

	commandList->close();
}

const agpu_ref<agpu_command_list> &AgpuCanvas::getCommandBundle()
{
	return commandList;
}

void AgpuCanvas::setColor(const glm::vec4 &color)
{
	currentColor = color;
}

void AgpuCanvas::drawLine(const glm::vec2 &p1, const glm::vec2 &p2)
{
	beginLineShape();
	addVertex(Vertex(transformPosition(p1), currentColor));
	addVertex(Vertex(transformPosition(p2), currentColor));
	addIndex(0);
	addIndex(1);
}

void AgpuCanvas::drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
	beginLineShape();
	addVertex(Vertex(transformPosition(p1), currentColor));
	addVertex(Vertex(transformPosition(p2), currentColor));
	addVertex(Vertex(transformPosition(p3), currentColor));
	addIndex(0);
	addIndex(1);
	addIndex(1);
	addIndex(2);
	addIndex(2);
	addIndex(0);
}

void AgpuCanvas::drawRectangle(const Rectangle &rectangle)
{
	beginLineShape();
	addVertex(Vertex(transformPosition(rectangle.getBottomLeft()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getBottomRight()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getTopRight()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getTopLeft()), currentColor));
	addIndex(0);
	addIndex(1);
	addIndex(1);
	addIndex(2);
	addIndex(2);
	addIndex(3);
	addIndex(3);
	addIndex(0);
}

void AgpuCanvas::drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
	beginTriangleShape();
	addVertex(Vertex(transformPosition(p1), currentColor));
	addVertex(Vertex(transformPosition(p2), currentColor));
	addVertex(Vertex(transformPosition(p3), currentColor));
	addIndex(0);
	addIndex(1);
	addIndex(2);
}

void AgpuCanvas::drawFillRectangle(const Rectangle &rectangle)
{
	beginTriangleShape();
	addVertex(Vertex(transformPosition(rectangle.getBottomLeft()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getBottomRight()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getTopRight()), currentColor));
	addVertex(Vertex(transformPosition(rectangle.getTopLeft()), currentColor));
	addIndex(0);
	addIndex(1);
	addIndex(2);
	addIndex(2);
	addIndex(3);
	addIndex(0);
}

void AgpuCanvas::beginLineShape()
{
	if(shapeType != ST_Line && shapeType != ST_Unknown)
		endSubmesh();

	if(shapeType != ST_Line)
		drawCommandsToAdd.push_back([this] {
			commandList->usePipelineState(linePipeline.get());
			commandList->setPrimitiveTopology(AGPU_LINES);
		});

	shapeType = ST_Line;
	baseVertex = (agpu_uint)vertices.size();
}

void AgpuCanvas::beginTriangleShape()
{
	if(shapeType != ST_Triangle && shapeType != ST_Unknown)
		endSubmesh();

	if(shapeType != ST_Triangle)
		drawCommandsToAdd.push_back([this] {
			commandList->usePipelineState(trianglePipeline.get());
			commandList->setPrimitiveTopology(AGPU_TRIANGLES);
		});

	shapeType = ST_Triangle;
	baseVertex = (agpu_uint)vertices.size();
}

void AgpuCanvas::endSubmesh()
{
	int start = startIndex;
	int count = (int)indices.size() - startIndex;
	if(!count)
		return;

	drawCommandsToAdd.push_back([=]{
		commandList->drawElements(count, 1, start, 0, 0);
	});
	startIndex = (int)indices.size();
}

void AgpuCanvas::addVertex(const AgpuCanvasVertex &vertex)
{
	vertices.push_back(vertex);
}

void AgpuCanvas::addIndex(int index)
{
	indices.push_back(index + baseVertex);
}

const glm::mat3 &AgpuCanvas::getTransform() const
{
	return transform;
}

void AgpuCanvas::setTransform(const glm::mat3 &newTransform)
{
	transform = newTransform;
}

} // End of namespace GUI
} // End of namespace Loden
