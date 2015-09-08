#ifndef LODEN_PIPELINE_BUILDER_HPP
#define LODEN_PIPELINE_BUILDER_HPP

#include "AGPU/agpu.hpp"
#include "Loden/Common.hpp"

namespace Loden
{

class PipelineStateManager;

/**
 * The pipeline builder
 */
class LODEN_CORE_EXPORT PipelineBuilder
{
public:
	PipelineBuilder(PipelineStateManager *manager, const agpu_ref<agpu_pipeline_builder> &builder);
	~PipelineBuilder();

	PipelineBuilder &setGeometryShader(const std::string &name);	
	PipelineBuilder &setFragmentShader(const std::string &name);
	PipelineBuilder &setVertexShader(const std::string &name);

	PipelineBuilder &setRenderTargetCount(int count);
	PipelineBuilder &setPrimitiveType(agpu_primitive_type type);
	PipelineBuilder &setVertexLayout(size_t vertexBufferCount, size_t layoutSize, agpu_vertex_attrib_description *layout);
	
	agpu_pipeline_state *finish();
	
private:
	PipelineStateManager *manager;
	agpu_ref<agpu_pipeline_builder> builder;
};
} // End of namespace Loden

#endif //LODEN_PIPELINE_BUILDER_HPP
