#ifndef LODEN_AGPU_CANVAS_HPP
#define LODEN_AGPU_CANVAS_HPP

#include "Loden/Common.hpp"
#include "Loden/GUI/Canvas.hpp"
#include "Loden/PipelineStateManager.hpp"
#include "AGPU/agpu.hpp"
#include <vector>
#include <glm/vec3.hpp>
#include <functional>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(AgpuCanvas);

struct AgpuCanvasVertex
{
	AgpuCanvasVertex() {}
	AgpuCanvasVertex(const glm::vec2 &position, const glm::vec4 &color)
		: position(position), color(color) {}
	
	glm::vec2 position;
	glm::vec2 texcoord;
	glm::vec4 color;
};

class AgpuCanvasPathProcessor;

/**
 * AGPU canvas
 */	
class LODEN_CORE_EXPORT AgpuCanvas: public Canvas
{
public:
	typedef AgpuCanvasVertex Vertex;
	
	~AgpuCanvas();
	
	static AgpuCanvasPtr create(const PipelineStateManagerPtr &stateManager);
	
	virtual void setColor(const glm::vec4 &color);
	
	virtual void drawLine(const glm::vec2 &p1, const glm::vec2 &p2);
	virtual void drawTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3);
	virtual void drawRectangle(const Rectangle &rectangle);
    virtual void drawRoundedRectangle(const Rectangle &rectangle, float cornerRadius);

	virtual void drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3);
	virtual void drawFillRectangle(const Rectangle &rectangle);
    virtual void drawFillRoundedRectangle(const Rectangle &rectangle, float cornerRadius);

    // Text drawing
    virtual glm::vec2 drawText(const std::string &text, int pointSize, glm::vec2 position);
    virtual glm::vec2 drawTextUtf16(const std::wstring &text, int pointSize, glm::vec2 position) ;

    // Fill paths.
    virtual void beginFillPath(PathFillRule fillRule = PathFillRule::EvenOdd);
    virtual void closePath();
    virtual void moveTo(const glm::vec2 &point);
    virtual void lineTo(const glm::vec2 &point);
    virtual void quadTo(const glm::vec2 &control, const glm::vec2 &point);
    virtual void cubicTo(const glm::vec2 &control, const glm::vec2 &control2, const glm::vec2 &point);
    virtual void endFillPath();

    // Stroke paths
    virtual void beginStrokePath();
    virtual void endStrokePath();

	virtual const glm::mat3 &getTransform() const;
	virtual void setTransform(const glm::mat3 &newTransform);

	void reset();
	void close();
	const agpu_ref<agpu_command_list> &getCommandBundle();
	
private:
	glm::vec2 transformPosition(const glm::vec2 &pos)
	{
		auto v3 = transform * glm::vec3(pos, 1.0);
		return glm::vec2(v3.x, v3.y);
	}
	
    void coverBox(const Rectangle &rectangle);

	void createVertexBuffer();
	void createIndexBuffer();
	
	enum ShapeType
	{
		ST_Unknown = -1,
		ST_Line,
		ST_Triangle
	};

	AgpuCanvas();

	void beginConvexLines();
	void beginConvexTriangles();
    void beginShapeWithPipeline(ShapeType newShapeType, agpu_pipeline_state *pipeline);

	void endSubmesh();
	void addVertex(const AgpuCanvasVertex &vertex);
    void addVertexPosition(const glm::vec2 &position);
	void addIndex(int index);

    // Current canvas state
	glm::vec4 currentColor;
	glm::mat3 transform;

    // Current font state
    FontFacePtr fontFace;

    // Buffer states
	size_t vertexCapacity;
	size_t indexCapacity;
	int startIndex;
	int baseVertex;
	ShapeType shapeType;
    agpu_pipeline_state *currentPipeline;
		
	agpu_ref<agpu_command_allocator> allocator;
	agpu_ref<agpu_command_list> commandList;
	
	PipelineStateManagerPtr stateManager;
	agpu_device_ref device;
	agpu_buffer_ref vertexBuffer;
	agpu_vertex_binding_ref vertexBufferBinding;
	
	agpu_buffer_ref indexBuffer;
    agpu_shader_signature_ref shaderSignature;
    
    agpu_pipeline_state_ref stencilNonZeroPipeline;
    agpu_pipeline_state_ref stencilEvenOddPipeline;
    agpu_pipeline_state_ref coverColorPipeline;

	agpu_pipeline_state_ref convexColorLinePipeline;
	agpu_pipeline_state_ref convexColorTrianglePipeline;

    // Path stenciling.
    agpu_pipeline_state_ref triangleStencilSetPipeline;

    // Path filling.
    agpu_pipeline_state_ref triangleStencilClearAndFillPipeline;

	std::vector<AgpuCanvasVertex> vertices;
	std::vector<int> indices;
	std::vector<std::function<void ()> > drawCommandsToAdd;

    // Path processing strategies.
    friend class AgpuCanvasPathProcessor;
    friend class AgpuConvexPathProcessor;
    friend class AgpuNoWidthStrokePathProcessor;
    friend class AgpuStencilPathProcessor;
    friend class AgpuStencilEvenOddPathProcessor;
    friend class AgpuStencilNonZeroPathProcessor;

    AgpuCanvasPathProcessor *currentPathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> nullPathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> noWithStrokePathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> strokePathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> convexPathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> evenOddRulePathProcessor;
    std::unique_ptr<AgpuCanvasPathProcessor> nonZeroRulePathProcessor;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_AGPU_CANVAS_HPP
