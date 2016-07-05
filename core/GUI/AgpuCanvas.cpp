#include "Loden/GUI/AgpuCanvas.hpp"
#include "Loden/GUI/Font.hpp"
#include "Loden/GUI/FontManager.hpp"
#include "Loden/Math.hpp"
#include "Loden/Printing.hpp"
#include <glm/gtx/norm.hpp>

namespace Loden
{
namespace GUI
{

const float CurveFlattnessFactor = 1.01f;
const float CurvePixelThreshold = 0.2f;

/**
 * Path processing strategy.
 */
class AgpuCanvasPathProcessor
{
public:
    AgpuCanvasPathProcessor(AgpuCanvas *canvas)
        : canvas(canvas)
    {
    }
    virtual ~AgpuCanvasPathProcessor()
    {
    }

    virtual void begin()
    {
    }

    virtual void closePath()
    {
    }

    virtual void moveTo(const glm::vec2 &point)
    {
    }

    virtual void lineTo(const glm::vec2 &point)
    {
    }

    virtual void quadTo(const glm::vec2 &control, const glm::vec2 &point)
    {
    }

    virtual void cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point)
    {
    }

    virtual void end()
    {
    }

    AgpuCanvas *canvas;
};

/**
* Triangle fan based path processor
*/
class AgpuSoftwareTessellationPathProcessor : public AgpuCanvasPathProcessor
{
public:
    AgpuSoftwareTessellationPathProcessor(AgpuCanvas *canvas)
        : AgpuCanvasPathProcessor(canvas)
    {
    }

    virtual void begin();
    virtual void end();

    virtual void closePath();
    virtual void moveTo(const glm::vec2 &point);
    virtual void lineTo(const glm::vec2 &point);
    virtual void quadTo(const glm::vec2 &control, const glm::vec2 &point);
    virtual void cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point);

    glm::vec2 closePosition;
    glm::vec2 currentPosition;
};

/**
* Triangle fan based path processor
*/
class AgpuConvexPathProcessor: public AgpuSoftwareTessellationPathProcessor
{
public:
    typedef AgpuSoftwareTessellationPathProcessor BaseClass;

    AgpuConvexPathProcessor(AgpuCanvas *canvas)
        : BaseClass(canvas)
    {
        vertexCount = 0;
    }


    virtual void begin();
    virtual void end();
    virtual void moveTo(const glm::vec2 &point);
    virtual void lineTo(const glm::vec2 &point);

    int vertexCount;
};

/**
* Concave path processor
*/
class AgpuStencilPathProcessor : public AgpuSoftwareTessellationPathProcessor
{
public:
    typedef AgpuSoftwareTessellationPathProcessor BaseClass;

    AgpuStencilPathProcessor(AgpuCanvas *canvas)
        : BaseClass(canvas)
    {
        totalVertexCount = 0;
        vertexCount = 0;
    }

    virtual agpu_pipeline_state *getPipelineState() const = 0;

    virtual void begin();
    virtual void end();
    virtual void moveTo(const glm::vec2 &point);
    virtual void lineTo(const glm::vec2 &point);

    int totalVertexCount;
    int vertexCount;
    Rectangle boundingBox;
};

/**
* Concave even-odd rule path processor
*/
class AgpuStencilEvenOddPathProcessor : public AgpuStencilPathProcessor
{
public:
    typedef AgpuStencilPathProcessor BaseClass;

    AgpuStencilEvenOddPathProcessor(AgpuCanvas *canvas)
        : BaseClass(canvas)
    {
    }

    agpu_pipeline_state *getPipelineState() const
    {
        return canvas->stencilEvenOddPipeline.get();
    }
};

/**
* Concave non-zero rule path processor
*/
class AgpuStencilNonZeroPathProcessor : public AgpuStencilPathProcessor
{
public:
    typedef AgpuStencilPathProcessor BaseClass;

    AgpuStencilNonZeroPathProcessor(AgpuCanvas *canvas)
        : BaseClass(canvas)
    {
    }

    agpu_pipeline_state *getPipelineState() const
    {
        return canvas->stencilNonZeroPipeline.get();
    }
};

/**
* No width stroke path processor
*/
class AgpuNoWidthStrokePathProcessor : public AgpuSoftwareTessellationPathProcessor
{
public:
    typedef AgpuSoftwareTessellationPathProcessor BaseClass;

    AgpuNoWidthStrokePathProcessor(AgpuCanvas *canvas)
        : BaseClass(canvas)
    {
        vertexCount = 0;
    }

    virtual void begin();
    virtual void end();
    virtual void moveTo(const glm::vec2 &point);
    virtual void lineTo(const glm::vec2 &point);

    int vertexCount;
};

/// AGPU Canvas.
AgpuCanvas::AgpuCanvas()
{
	vertexCapacity = 0;
	indexCapacity = 0;
    currentPipeline = nullptr;
    currentTextureBinding = nullptr;
    currentFontBinding = nullptr;
    coveringType = CT_Draw;
    usingBundle = false;

    nullPathProcessor.reset(new AgpuCanvasPathProcessor(this));
    currentPathProcessor = nullPathProcessor.get();

    noWithStrokePathProcessor.reset(new AgpuNoWidthStrokePathProcessor(this));
    strokePathProcessor.reset(new AgpuNoWidthStrokePathProcessor(this)); // TODO: Implement this properly
    convexPathProcessor.reset(new AgpuConvexPathProcessor(this));
    evenOddRulePathProcessor.reset(new AgpuStencilEvenOddPathProcessor(this));
    nonZeroRulePathProcessor.reset(new AgpuStencilNonZeroPathProcessor(this));
}

AgpuCanvas::~AgpuCanvas()
{
}

AgpuCanvasPtr AgpuCanvas::create(const PipelineStateManagerPtr &stateManager, bool usingBundle)
{
	auto &device = stateManager->getDevice();

	auto layout = stateManager->getVertexLayout("CanvasVertex2D");
	if(!layout)
		return nullptr;

	// Create the command list allocator.
	auto allocator = device->createCommandAllocator(AGPU_COMMAND_LIST_TYPE_BUNDLE, stateManager->getEngine()->getGraphicsCommandQueue().get());
	if(!allocator)
		return nullptr;

	// Create the command list.
    agpu_command_list_ref bundleCommandList;
    if(usingBundle)
    {
    	bundleCommandList = device->createCommandList(AGPU_COMMAND_LIST_TYPE_BUNDLE, allocator, nullptr);
    	if(!bundleCommandList)
    		return nullptr;
    	bundleCommandList->close();
    }

	// Create the canvas object.
	auto canvas = AgpuCanvasPtr(new AgpuCanvas());
    canvas->usingBundle = usingBundle;
	canvas->stateManager = stateManager;
	canvas->device = device;
	canvas->allocator = allocator;
	canvas->bundleCommandList = bundleCommandList;
	canvas->vertexBufferBinding = device->createVertexBinding(layout.get());
    canvas->shaderSignature = stateManager->getShaderSignature("GUI");
	canvas->convexColorLinePipeline = stateManager->getPipelineState("canvas2d.polygon.convex.color.lines.blend.over");
    assert(canvas->convexColorLinePipeline);

	canvas->convexColorTrianglePipeline = stateManager->getPipelineState("canvas2d.polygon.convex.color.triangles.blend.over");
    assert(canvas->convexColorTrianglePipeline);

    canvas->stencilNonZeroPipeline = stateManager->getPipelineState("canvas2d.polygon.stencil.non-zero");
    assert(canvas->stencilNonZeroPipeline);

    canvas->stencilEvenOddPipeline = stateManager->getPipelineState("canvas2d.polygon.stencil.even-odd");
    assert(canvas->stencilEvenOddPipeline);

    canvas->coverColorPipeline = stateManager->getPipelineState("canvas2d.polygon.cover.color");
    assert(canvas->coverColorPipeline);

    canvas->textColorPipeline = stateManager->getPipelineState("canvas2d.text.color");
    assert(canvas->textColorPipeline);

    canvas->textSdfColorPipeline = stateManager->getPipelineState("canvas2d.textsdf.color");
    assert(canvas->textSdfColorPipeline);

    agpu_sampler_description samplerDesc;
    memset(&samplerDesc, 0, sizeof(samplerDesc));
    samplerDesc.filter = AGPU_FILTER_MIN_LINEAR_MAG_LINEAR_MIPMAP_NEAREST;
    samplerDesc.address_u = AGPU_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.address_v = AGPU_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.address_w = AGPU_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.max_lod = 10000.0;
    canvas->sampler = canvas->shaderSignature->createShaderResourceBinding(3);
    canvas->sampler->createSampler(0, &samplerDesc);
    canvas->sampler->createSampler(1, &samplerDesc);

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
    currentPipeline = nullptr;
    currentTextureBinding = nullptr;
    currentFontBinding = nullptr;
	drawCommandsToAdd.clear();
    coveringType = CT_Draw;

	shapeType = ST_Unknown;
	vertices.clear();
	indices.clear();
	allocator->reset();
    currentPathProcessor = nullPathProcessor.get();

    // Use the default font face.
    {
        auto defaultFont = stateManager->getEngine()->getFontManager()->getDefaultFont();
        if (defaultFont)
            fontFace = defaultFont->getDefaultFace();
    }

}

void AgpuCanvas::close()
{
	if(vertices.empty() || indices.empty())
	   return;
	endSubmesh();

	if(vertexCapacity < vertices.size())
		createVertexBuffer();
	if(indexCapacity < indices.size())
		createIndexBuffer();
	vertexBuffer->uploadBufferData(0, vertices.size()*sizeof(Vertex), &vertices[0]);
	indexBuffer->uploadBufferData(0, indices.size()*sizeof(int), &indices[0]);

    // Set the shader signature.
    if(usingBundle)
    {
        bundleCommandList->reset(allocator.get(), nullptr);
        bundleCommandList->setStencilReference(0);

        emitCommandsInto(bundleCommandList);
        bundleCommandList->close();
    }
}

void AgpuCanvas::emitCommandsInto(agpu_command_list_ref &commandList)
{
    commandList->setShaderSignature(shaderSignature.get());
    commandList->useShaderResources(sampler.get());

	// Store the commands in the command list.
	commandList->useVertexBinding(vertexBufferBinding.get());
	commandList->useIndexBuffer(indexBuffer.get());
	for(auto &command : drawCommandsToAdd)
		command(commandList);
}

const agpu_ref<agpu_command_list> &AgpuCanvas::getCommandBundle()
{
	return bundleCommandList;
}

void AgpuCanvas::setColor(const glm::vec4 &color)
{
	currentColor = color;
}

void AgpuCanvas::drawLine(const glm::vec2 &p1, const glm::vec2 &p2)
{
    beginConvexLines();
	addVertex(Vertex(transformPosition(p1), currentColor));
	addVertex(Vertex(transformPosition(p2), currentColor));
	addIndex(0);
	addIndex(1);
}

void AgpuCanvas::drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
    beginConvexTriangles();
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
    beginConvexLines();
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

void AgpuCanvas::drawRoundedRectangle(const Rectangle &rectangle, float cornerRadius)
{
    glm::vec2 dx(cornerRadius, 0);
    glm::vec2 dy(0, cornerRadius);

    beginStrokePath();
    moveTo(rectangle.getBottomLeft() + dx);
    lineTo(rectangle.getBottomRight() - dx);
    quadTo(rectangle.getBottomRight(), rectangle.getBottomRight() + dy);
    lineTo(rectangle.getTopRight() - dy);
    quadTo(rectangle.getTopRight(), rectangle.getTopRight() - dx);
    lineTo(rectangle.getTopLeft() + dx);
    quadTo(rectangle.getTopLeft(), rectangle.getTopLeft() - dy);
    lineTo(rectangle.getBottomLeft() + dy);
    quadTo(rectangle.getBottomLeft(), rectangle.getBottomLeft() + dx);
    endStrokePath();
}

void AgpuCanvas::drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
	beginConvexLines();
	addVertex(Vertex(transformPosition(p1), currentColor));
	addVertex(Vertex(transformPosition(p2), currentColor));
	addVertex(Vertex(transformPosition(p3), currentColor));
	addIndex(0);
	addIndex(1);
	addIndex(2);
}

void AgpuCanvas::drawFillRectangle(const Rectangle &rectangle)
{
    beginConvexTriangles();
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

void AgpuCanvas::drawFillRoundedRectangle(const Rectangle &rectangle, float cornerRadius)
{
    glm::vec2 dx(cornerRadius, 0);
    glm::vec2 dy(0, cornerRadius);

    beginFillPath(PathFillRule::Convex);
    moveTo(rectangle.getBottomLeft() + dx);
    lineTo(rectangle.getBottomRight() - dx);
    quadTo(rectangle.getBottomRight(), rectangle.getBottomRight() + dy);
    lineTo(rectangle.getTopRight() - dy);
    quadTo(rectangle.getTopRight(), rectangle.getTopRight() - dx);
    lineTo(rectangle.getTopLeft() + dx);
    quadTo(rectangle.getTopLeft(), rectangle.getTopLeft() - dy);
    lineTo(rectangle.getBottomLeft() + dy);
    quadTo(rectangle.getBottomLeft(), rectangle.getBottomLeft() + dx);
    endFillPath();
}

// Text drawing
glm::vec2 AgpuCanvas::drawText(const std::string &text, int pointSize, glm::vec2 position)
{
    if (!fontFace)
        return position;
    return fontFace->drawUtf8(this, text, pointSize, position);
}

glm::vec2 AgpuCanvas::drawTextUtf16(const std::wstring &text, int pointSize, glm::vec2 position)
{
    if (!fontFace)
        return position;
    return fontFace->drawUtf16(this, text, pointSize, position);
}

void AgpuCanvas::beginBitmapTextDrawing(void *binding, bool distanceField)
{
    if(distanceField)
        beginShapeWithPipeline(ST_Triangle, textSdfColorPipeline.get(), nullptr, (agpu_shader_resource_binding*)binding);
    else
        beginShapeWithPipeline(ST_Triangle, textColorPipeline.get(), nullptr, (agpu_shader_resource_binding*) binding);
}

void AgpuCanvas::withNewBaseVertex()
{
    baseVertex = (agpu_uint)vertices.size();
}

void AgpuCanvas::drawBitmapCharacter(const Rectangle &destRectangle, Rectangle &sourceRectangle)
{
    withNewBaseVertex();
    addVertex(Vertex(transformPosition(destRectangle.getBottomLeft()), sourceRectangle.getBottomLeft(), currentColor));
    addVertex(Vertex(transformPosition(destRectangle.getBottomRight()), sourceRectangle.getBottomRight(), currentColor));
    addVertex(Vertex(transformPosition(destRectangle.getTopRight()), sourceRectangle.getTopRight(), currentColor));
    addVertex(Vertex(transformPosition(destRectangle.getTopLeft()), sourceRectangle.getTopLeft(), currentColor));
    addIndex(0);
    addIndex(1);
    addIndex(2);
    addIndex(2);
    addIndex(3);
    addIndex(0);
}

void AgpuCanvas::endBitmapTextDrawing()
{
}

// Covering
void AgpuCanvas::coverBox(const Rectangle &rectangle)
{
    switch (coveringType)
    {
    case CT_Draw:
        beginShapeWithPipeline(ST_Triangle, coverColorPipeline.get());
        break;
    case CT_ClipPath:
        beginShapeWithPipeline(ST_Triangle, coverColorPipeline.get());
        break;
    case CT_ClipPathPop:
        beginShapeWithPipeline(ST_Triangle, coverColorPipeline.get());
        break;
    }

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

// Fill paths.
void AgpuCanvas::beginFillPath(PathFillRule fillRule)
{
    switch (fillRule)
    {
    case PathFillRule::EvenOdd:
        currentPathProcessor = evenOddRulePathProcessor.get();
        break;
    case PathFillRule::NonZero:
        currentPathProcessor = nonZeroRulePathProcessor.get();
        break;
    case PathFillRule::Convex:
        currentPathProcessor = convexPathProcessor.get();
        break;
    }
    coveringType = CT_Draw;
    currentPathProcessor->begin();
}

void AgpuCanvas::endFillPath()
{
    currentPathProcessor->end();
    currentPathProcessor = nullPathProcessor.get();
}

void AgpuCanvas::beginClipPath(PathFillRule fillRule)
{
    beginFillPath();
    coveringType = CT_ClipPath;
}

void AgpuCanvas::endClipPath()
{
    currentPathProcessor->end();
    currentPathProcessor = nullPathProcessor.get();
    coveringType = CT_Draw;
}

void AgpuCanvas::popClipPath()
{

}

void AgpuCanvas::closePath()
{
    currentPathProcessor->closePath();
}

void AgpuCanvas::moveTo(const glm::vec2 &point)
{
    currentPathProcessor->moveTo(point);
}

void AgpuCanvas::lineTo(const glm::vec2 &point)
{
    currentPathProcessor->lineTo(point);
}

void AgpuCanvas::quadTo(const glm::vec2 &control, const glm::vec2 &point)
{
    currentPathProcessor->quadTo(control, point);
}

void AgpuCanvas::cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point)
{
    currentPathProcessor->cubicTo(control, control2, point);
}

// Stroke paths
void AgpuCanvas::beginStrokePath()
{
    currentPathProcessor = strokePathProcessor.get();
    currentPathProcessor->begin();
}

void AgpuCanvas::endStrokePath()
{
    currentPathProcessor->end();
    currentPathProcessor = nullPathProcessor.get();
}

void AgpuCanvas::beginConvexLines()
{
    beginShapeWithPipeline(ST_Line, convexColorLinePipeline.get());
}

void AgpuCanvas::beginConvexTriangles()
{
    beginShapeWithPipeline(ST_Triangle, convexColorTrianglePipeline.get());
}

void AgpuCanvas::beginShapeWithPipeline(ShapeType newShapeType, agpu_pipeline_state *pipeline, agpu_shader_resource_binding *textureBinding, agpu_shader_resource_binding *fontBinding)
{
    if ((shapeType != newShapeType && shapeType != ST_Unknown) ||
        (pipeline != currentPipeline && currentPipeline != nullptr) ||
        (currentTextureBinding != textureBinding && textureBinding != nullptr) ||
        (currentFontBinding != fontBinding && fontBinding != nullptr))
        endSubmesh();

    if (currentPipeline != pipeline)
    {
        drawCommandsToAdd.push_back([=] (agpu_command_list_ref &commandList) {
            commandList->usePipelineState(pipeline);
        });
        currentPipeline = pipeline;
    }

    if (currentTextureBinding != textureBinding && textureBinding != nullptr)
    {
        drawCommandsToAdd.push_back([=] (agpu_command_list_ref &commandList) {
            commandList->useShaderResources(textureBinding);
        });
        currentTextureBinding = textureBinding;
    }

    if (currentFontBinding != fontBinding && fontBinding != nullptr)
    {
        drawCommandsToAdd.push_back([=] (agpu_command_list_ref &commandList) {
            commandList->useShaderResources(fontBinding);
        });
        currentFontBinding = fontBinding;
    }

    shapeType = newShapeType;
    baseVertex = (agpu_uint)vertices.size();
}

void AgpuCanvas::endSubmesh()
{
	int start = startIndex;
	int count = (int)indices.size() - startIndex;
	if(!count)
		return;

	drawCommandsToAdd.push_back([=] (agpu_command_list_ref &commandList) {
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

void AgpuCanvas::addVertexPosition(const glm::vec2 &position)
{
    addVertex(Vertex(transformPosition(position), currentColor));
}

const glm::mat3 &AgpuCanvas::getTransform() const
{
	return transform;
}

void AgpuCanvas::setTransform(const glm::mat3 &newTransform)
{
	transform = newTransform;
}

// Software tesselation path processor
void AgpuSoftwareTessellationPathProcessor::begin()
{
    currentPosition = glm::vec2();
    closePosition = glm::vec2();
}

void AgpuSoftwareTessellationPathProcessor::end()
{
}

void AgpuSoftwareTessellationPathProcessor::closePath()
{
    lineTo(closePosition);
}

void AgpuSoftwareTessellationPathProcessor::moveTo(const glm::vec2 &point)
{
    currentPosition = point;
    closePosition = point;
}

void AgpuSoftwareTessellationPathProcessor::lineTo(const glm::vec2 &point)
{
    currentPosition = point;
}

void AgpuSoftwareTessellationPathProcessor::quadTo(const glm::vec2 &control, const glm::vec2 &point)
{
    auto lineLength = glm::length(point - currentPosition);
    auto arcLength = glm::length(control - currentPosition) + glm::length(point - control);

    auto delta = arcLength - lineLength;
    if (arcLength > CurveFlattnessFactor * lineLength && delta > CurvePixelThreshold)
    {
        auto m1 = midpoint(currentPosition, control);
        auto m2 = midpoint(control, point);
        auto m3 = midpoint(m1, m2);
        quadTo(m1, m3);
        quadTo(m2, point);
    }
    else
    {
        lineTo(point);
    }
}

void AgpuSoftwareTessellationPathProcessor::cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point)
{
    auto lineLength = glm::length(point - currentPosition);
    auto arcLength = glm::length(control - currentPosition) + glm::length(point - control);

    auto delta = arcLength - lineLength;
    if (arcLength > CurveFlattnessFactor * lineLength && delta > CurvePixelThreshold)
    {
        auto m1 = midpoint(currentPosition, control);
        auto m2 = midpoint(control, control2);
        auto m3 = midpoint(control2, point);

        auto m4 = midpoint(m1, m2);
        auto m5 = midpoint(m2, m3);
        auto m6 = midpoint(m4, m5);

        cubicTo(m1, m4, m6);
        cubicTo(m2, m5, point);
    }
    else
    {
        lineTo(point);
    }
}

// Triangle fan based path processor
void AgpuConvexPathProcessor::begin()
{
    BaseClass::begin();
    vertexCount = 0;
}

void AgpuConvexPathProcessor::end()
{
    BaseClass::end();
}

void AgpuConvexPathProcessor::moveTo(const glm::vec2 &point)
{
    BaseClass::moveTo(point);
    vertexCount = 0;
}

void AgpuConvexPathProcessor::lineTo(const glm::vec2 &point)
{
    if (vertexCount == 0)
    {
        canvas->beginConvexTriangles();
        canvas->addVertexPosition(currentPosition);
        ++vertexCount;
    }

    currentPosition = point;
    canvas->addVertexPosition(currentPosition);
    ++vertexCount;

    if (vertexCount > 2)
    {
        canvas->addIndex(0);
        canvas->addIndex(vertexCount - 2);
        canvas->addIndex(vertexCount - 1);
    }
}

// Stencil path processor
void AgpuStencilPathProcessor::begin()
{
    BaseClass::begin();
    vertexCount = 0;
    totalVertexCount = 0;
}

void AgpuStencilPathProcessor::end()
{
    BaseClass::end();
    if (!totalVertexCount)
        return;
    canvas->coverBox(boundingBox);
}

void AgpuStencilPathProcessor::moveTo(const glm::vec2 &point)
{
    BaseClass::moveTo(point);
    vertexCount = 0;
}

void AgpuStencilPathProcessor::lineTo(const glm::vec2 &point)
{
    if (totalVertexCount == 0)
        boundingBox.min = boundingBox.max = currentPosition;

    if (vertexCount == 0)
    {
        canvas->beginShapeWithPipeline(AgpuCanvas::ST_Triangle, getPipelineState());
        canvas->addVertexPosition(currentPosition);
        ++vertexCount;
        ++totalVertexCount;
    }

    currentPosition = point;
    canvas->addVertexPosition(currentPosition);
    boundingBox.insertPoint(point);
    ++vertexCount;
    ++totalVertexCount;

    if (vertexCount > 2)
    {
        canvas->addIndex(0);
        canvas->addIndex(vertexCount - 2);
        canvas->addIndex(vertexCount - 1);
    }
}

// No width stroke path processor
void AgpuNoWidthStrokePathProcessor::begin()
{
    BaseClass::begin();
    vertexCount = 0;
}

void AgpuNoWidthStrokePathProcessor::end()
{
    BaseClass::end();
}

void AgpuNoWidthStrokePathProcessor::moveTo(const glm::vec2 &point)
{
    BaseClass::moveTo(point);
    vertexCount = 0;
}

void AgpuNoWidthStrokePathProcessor::lineTo(const glm::vec2 &point)
{
    if (vertexCount == 0)
    {
        canvas->beginConvexLines();
        canvas->addVertexPosition(currentPosition);
        ++vertexCount;
    }

    currentPosition = point;
    canvas->addVertexPosition(currentPosition);

    ++vertexCount;
    canvas->addIndex(vertexCount - 1);
    canvas->addIndex(vertexCount - 2);
}

} // End of namespace GUI
} // End of namespace Loden
