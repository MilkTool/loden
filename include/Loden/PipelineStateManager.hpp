#ifndef LODEN_PIPELINE_STATE_MANAGER_HPP
#define LODEN_PIPELINE_STATE_MANAGER_HPP

#include "Loden/Common.hpp"
#include "Loden/PipelineStateFactory.hpp"
#include <unordered_map>
#include <map>

namespace Loden
{

LODEN_DECLARE_CLASS(PipelineStateManager);

/**
 * Pipeline state manager
 */
class LODEN_CORE_EXPORT PipelineStateManager
{
public:
	PipelineStateManager(agpu_ref<agpu_device> device);
	~PipelineStateManager();

    bool initialize();

	const agpu_ref<agpu_device> &getDevice() const;

    agpu_ref<agpu_shader_signature> getShaderSignature(const std::string &name);
    void addShaderSignature(const std::string &name, const agpu_ref<agpu_shader_signature> &shaderSignature);

	agpu_ref<agpu_vertex_layout> getVertexLayout(size_t vertexBufferCount, size_t layoutSize, agpu_vertex_attrib_description *layout);

	agpu_ref<agpu_shader> getShaderFromFile(const std::string &fileName, agpu_shader_type type);
	agpu_ref<agpu_pipeline_state> getState(PipelineStateFactory &factory);


private:
	agpu_ref<agpu_shader> compileShaderFromFile(const std::string &fileName, agpu_shader_type type);

	agpu_ref<agpu_device> device;
	std::map<std::pair<std::string, agpu_shader_type>, agpu_ref<agpu_shader>> shaders;
	std::unordered_map<PipelineStateFactory*, agpu_ref<agpu_pipeline_state>> states;
	std::map<std::pair<agpu_vertex_attrib_description*, std::pair<size_t, size_t>>, agpu_ref<agpu_vertex_layout>> vertexLayouts;
    std::map<std::string, agpu_ref<agpu_shader_signature>> shaderSignatures;

	agpu_shader_language preferredShaderLanguage;
};

}; // End of namespace Loden

#endif //LODEN_PIPELINE_STATE_MANAGER_HPP
