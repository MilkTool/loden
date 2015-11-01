#include "Loden/PipelineStateManager.hpp"
#include "Loden/PipelineBuilder.hpp"

namespace Loden
{

PipelineBuilder::PipelineBuilder(PipelineStateManager *manager, const agpu_ref<agpu_pipeline_builder> &builder)
	: manager(manager), builder(builder)
{
}

PipelineBuilder::~PipelineBuilder()
{
}

PipelineBuilder &PipelineBuilder::setShaderSignatureNamed(const std::string &name)
{
    builder->setShaderSignature(manager->getShaderSignature(name).get());
    return *this;
}

PipelineBuilder &PipelineBuilder::setGeometryShader(const std::string &name)
{
	builder->attachShader(manager->getShaderFromFile(name, AGPU_GEOMETRY_SHADER).get());
	return *this;
}

PipelineBuilder &PipelineBuilder::setFragmentShader(const std::string &name)
{
	builder->attachShader(manager->getShaderFromFile(name, AGPU_FRAGMENT_SHADER).get());
	return *this;
}

PipelineBuilder &PipelineBuilder::setVertexShader(const std::string &name)
{
	builder->attachShader(manager->getShaderFromFile(name, AGPU_VERTEX_SHADER).get());
	return *this;
}

PipelineBuilder &PipelineBuilder::setRenderTargetCount(int count)
{
	builder->setRenderTargetCount(count);
	return *this;
}

PipelineBuilder &PipelineBuilder::setPrimitiveType(agpu_primitive_type type)
{
	builder->setPrimitiveType(type);
	return *this;
}

PipelineBuilder &PipelineBuilder::setVertexLayout(size_t vertexBufferCount, size_t layoutSize, agpu_vertex_attrib_description *layout)
{
	auto vlayout = manager->getVertexLayout(vertexBufferCount, layoutSize, layout);
	builder->setVertexLayout(vlayout.get());
	return *this;
}

agpu_pipeline_state *PipelineBuilder::finish()
{
	auto result = builder->build();
	if(!result)
	{

	}

	return result;
}

}
