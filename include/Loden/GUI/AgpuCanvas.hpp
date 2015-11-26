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

	virtual void drawFillTriangle(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3);
	virtual void drawFillRectangle(const Rectangle &rectangle);

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
	
	void createVertexBuffer();
	void createIndexBuffer();
	
	enum ShapeType
	{
		ST_Unknown = -1,
		ST_Line,
		ST_Triangle
	};

		
	AgpuCanvas();

	void beginLineShape();
	void beginTriangleShape();
	void endSubmesh();
	void addVertex(const AgpuCanvasVertex &vertex);
	void addIndex(int index);

	glm::vec4 currentColor;
	glm::mat3 transform;
	size_t vertexCapacity;
	size_t indexCapacity;
	int startIndex;
	int baseVertex;
	ShapeType shapeType;
		
	agpu_ref<agpu_command_allocator> allocator;
	agpu_ref<agpu_command_list> commandList;
	
	PipelineStateManagerPtr stateManager;
	agpu_ref<agpu_device> device;
	agpu_ref<agpu_buffer> vertexBuffer;
	agpu_ref<agpu_vertex_binding> vertexBufferBinding;
	
	agpu_ref<agpu_buffer> indexBuffer;
    agpu_ref<agpu_shader_signature> shaderSignature;
	agpu_ref<agpu_pipeline_state> linePipeline;
	agpu_ref<agpu_pipeline_state> trianglePipeline;
	
	std::vector<AgpuCanvasVertex> vertices;
	std::vector<int> indices;
	std::vector<std::function<void ()> > drawCommandsToAdd;

};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_AGPU_CANVAS_HPP
