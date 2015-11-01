#include "Loden/GUI/AgpuCanvas.hpp"
#include "Loden/PipelineStateFactory.hpp"

namespace Loden
{
namespace GUI
{

agpu_vertex_attrib_description AgpuCanvasVertex::Description[] = {
    {0, 0, AGPU_FLOAT, 2, 1, false, offsetof(AgpuCanvasVertex, position)},
    {0, 1, AGPU_FLOAT, 2, 1, false, offsetof(AgpuCanvasVertex, texcoord)},
	{0, 2, AGPU_FLOAT, 4, 1, false, offsetof(AgpuCanvasVertex, color)},
};

const int AgpuCanvasVertex::DescriptionSize = 3;

static PipelineStateFactory canvasLinePipeline("AgpuCanvas::line", [](PipelineBuilder &builder) {
	builder
        .setShaderSignatureNamed("GUI")
		.setVertexShader("shaders/canvas2d/colorVertex")
		.setFragmentShader("shaders/canvas2d/colorFragment")
		.setPrimitiveType(AGPU_PRIMITIVE_TYPE_LINE)
		.setRenderTargetCount(1)
		.setVertexLayout(1, AgpuCanvasVertex::DescriptionSize, AgpuCanvasVertex::Description);
});
static PipelineStateFactory canvasTrianglePipeline("AgpuCanvas::triangle", [](PipelineBuilder &builder) {
	builder
        .setShaderSignatureNamed("GUI")
		.setVertexShader("shaders/canvas2d/colorVertex")
		.setFragmentShader("shaders/canvas2d/colorFragment")
		.setPrimitiveType(AGPU_PRIMITIVE_TYPE_TRIANGLE)
		.setRenderTargetCount(1)
		.setVertexLayout(1, AgpuCanvasVertex::DescriptionSize, AgpuCanvasVertex::Description);
});

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

	auto layout = stateManager->getVertexLayout(1, AgpuCanvasVertex::DescriptionSize, AgpuCanvasVertex::Description);
	if(!layout)
		return nullptr;

	// Create the command list allocator.
	auto allocator = device->createCommandAllocator();
	if(!allocator)
		return nullptr;

	// Create the command list.
	auto commandList = device->createCommandListBundle(allocator, nullptr);
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
	canvas->linePipeline = stateManager->getState(canvasLinePipeline);
	canvas->trianglePipeline = stateManager->getState(canvasTrianglePipeline);

	return canvas;
}

void AgpuCanvas::createVertexBuffer()
{
	vertexCapacity = vertices.size();

    agpu_buffer_description desc;
    desc.size = vertexCapacity*sizeof(Vertex);
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
    desc.size = indexCapacity*sizeof(int);
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
	vertexCapacity = 0;
	indexCapacity = 0;
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
	baseVertex = vertices.size();
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
	baseVertex = vertices.size();
}

void AgpuCanvas::endSubmesh()
{
	int start = startIndex;
	int count = indices.size() - startIndex;
	if(!count)
		return;

	drawCommandsToAdd.push_back([=]{
		commandList->drawElements(count, 1, start, 0, 0);
	});
	startIndex = indices.size();
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
