#ifndef LODEN_PIPELINE_STATE_FACTORY_HPP
#define LODEN_PIPELINE_STATE_FACTORY_HPP

#include "Loden/PipelineBuilder.hpp"

namespace Loden
{

/**
 * The pipeline state factory
 */
class PipelineStateFactory
{
public:
	PipelineStateFactory(const std::string &name, const std::function<void (PipelineBuilder&)> &factoryMethod)
		: name(name), factoryMethod(factoryMethod)
	{
		
	}
	
	const std::string &getName() const
	{
		return name;
	}

	agpu_pipeline_state *build(PipelineBuilder &builder) const
	{
		factoryMethod(builder);
		return builder.finish();
	}
	
private:
	std::string name;
	std::function<void (PipelineBuilder&)> factoryMethod;
};

}; // End of namespace Loden

#endif //LODEN_PIPELINE_STATE_FACTORY_HPP
