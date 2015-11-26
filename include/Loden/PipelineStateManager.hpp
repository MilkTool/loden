#ifndef LODEN_PIPELINE_STATE_MANAGER_HPP
#define LODEN_PIPELINE_STATE_MANAGER_HPP

#include "Loden/Common.hpp"
#include "Loden/JSON.hpp"
#include "AGPU/agpu.hpp"
#include <functional>
#include <map>
#include <vector>

namespace Loden
{

LODEN_DECLARE_CLASS(PipelineStateManager);
LODEN_DECLARE_CLASS(PipelineStateTemplate);

/**
 * A set of shaders.
 */
class ShaderSet
{
public:
    void addStage(const agpu_shader_ref &shader)
    {
        stages.push_back(shader);
    }

    std::vector<agpu_shader_ref> stages;
};

/**
 * Vertex attribute type
 */
class VertexAttributeType
{
public:
    VertexAttributeType() {}
    VertexAttributeType(agpu_field_type type, agpu_uint components, agpu_uint rows, agpu_bool normalized, agpu_uint size, agpu_uint alignment)
        : type(type), components(components), rows(rows), normalized(normalized), size(size), alignment(alignment) {}

    agpu_field_type type;
    agpu_uint components;
    agpu_uint rows;
    agpu_bool normalized;
    agpu_uint size;
    agpu_uint alignment;
};

/**
 * Pipeline state template.
 */
class PipelineStateTemplate
{
public:
    typedef std::function<bool(const agpu_pipeline_builder_ref &builder) > BuildAction;

    PipelineStateTemplate()
        : isAbstract(false)
    {
    }

    bool instantiateOn(const agpu_pipeline_builder_ref &builder)
    {
        if (parent)
        {
            if (!parent->instantiateOn(builder))
                return false;
        }

        for (auto &action : buildActions)
        {
            if (!action(builder))
                return false;
        }

        return true;
    }

    void addAction(const BuildAction &action)
    {
        buildActions.push_back(action);
    }

    bool isAbstract;
    std::string namePrefix;
    PipelineStateTemplatePtr parent;
    std::vector<BuildAction> buildActions;
};

/**
 * Pipeline state manager
 */
class LODEN_CORE_EXPORT PipelineStateManager
{
public:
	PipelineStateManager(const agpu_device_ref &device);
	~PipelineStateManager();

    bool initialize();
    bool loadStatesFromFile(const std::string &filename);
    bool loadShaderSignaturesFromFile(const std::string &filename);
    bool loadVertexLayoutsFromFile(const std::string &filename);
    bool loadShadersFromFile(const std::string &filename, const std::string &namePrefix = std::string());
    bool loadPipelineStatesFromFile(const std::string &filename, const std::string &namePrefix = std::string());
    

	const agpu_device_ref &getDevice() const;

    void addShaderSignature(const std::string &name, const agpu_shader_signature_ref &shaderSignature);
    void addVertexLayout(const std::string &name, const agpu_vertex_layout_ref &vertexLayout);
    void addShaderSet(const std::string &name, const ShaderSet &shaderSet);
    void addPipelineStateTemplate(const std::string &name, const PipelineStateTemplatePtr &stateTemplate);
    void addPipelineState(const std::string &name, const agpu_pipeline_state_ref &state);

    agpu_shader_signature_ref getShaderSignature(const std::string &name);
	agpu_vertex_layout_ref getVertexLayout(const std::string &name);
    ShaderSet *getShaderSet(const std::string &name);
	agpu_pipeline_state_ref getPipelineState(const std::string &name);
    PipelineStateTemplatePtr getPipelineStateTemplate(const std::string &name);

private:
    typedef std::function<bool (PipelineStateTemplate &, rapidjson::Value &)> PipelineStateTemplateParseAction;

    void buildParseTables();
    void buildPipelineStateParsingActions();

	agpu_shader_ref compileShaderFromFile(const std::string &fileName, agpu_shader_language language, agpu_shader_type type);

	agpu_device_ref device;
	std::map<std::string, agpu_pipeline_state_ref> pipelineStates;
	std::map<std::string, agpu_vertex_layout_ref> vertexLayouts;
    std::map<std::string, agpu_shader_signature_ref> shaderSignatures;
    std::map<std::string, ShaderSet> shaderSets;

    // Tables for parsing the state data.
    std::map<std::string, agpu_shader_binding_type> shaderBindingTypeNameMap;
    std::map<std::string, VertexAttributeType> vertexLayoutTypeMap;
    std::map<std::string, agpu_shader_type> shaderTypeNameMap;
    std::vector<std::pair<agpu_shader_language, std::string>> shaderLanguageSearchOrder;
    std::map<std::string, PipelineStateTemplateParseAction> pipelineStateParsingActions;
    std::map<std::string, PipelineStateTemplatePtr> pipelineStateTemplates;
    std::map<std::string, agpu_primitive_type> primitiveTypeNameMap;
};

}; // End of namespace Loden

#endif //LODEN_PIPELINE_STATE_MANAGER_HPP
