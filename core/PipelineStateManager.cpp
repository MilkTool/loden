#include "Loden/PipelineStateManager.hpp"
#include "Loden/Printing.hpp"
#include <vector>

namespace Loden
{

std::string readWholeFile(const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if(!file)
    {
        printError("Failed to open file %s\n", fileName.c_str());
        return std::string();
    }

    // Allocate the data.
    std::vector<char> data;
    fseek(file, 0, SEEK_END);
    data.resize(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Read the file
    if(fread(&data[0], data.size(), 1, file) != 1)
    {
        printError("Failed to read file %s\n", fileName.c_str());
        fclose(file);
        return std::string();
    }

    fclose(file);
    return std::string(data.begin(), data.end());
}

PipelineStateManager::PipelineStateManager(agpu_ref<agpu_device> device)
	: device(device)
{
    preferredShaderLanguage = device->getPreferredHighLevelShaderLanguage();
}

PipelineStateManager::~PipelineStateManager()
{
}

bool PipelineStateManager::initialize()
{
    // Create the GUI shader signature
    {
        // Create the GUI shader signature
        agpu_ref<agpu_shader_signature_builder> signatureBuilder = device->createShaderSignatureBuilder();
        signatureBuilder->addBindingBank(AGPU_SHADER_BINDING_TYPE_CBV, 1, 3);
        agpu_ref<agpu_shader_signature> shaderSignature = signatureBuilder->build();
        if(!shaderSignature)
        {
            printError("Failed to create the shader signature\n");
            return false;
        }

        addShaderSignature("GUI", shaderSignature);
    }

    return true;
}

const agpu_ref<agpu_device> &PipelineStateManager::getDevice() const
{
    return device;
}

agpu_ref<agpu_shader_signature> PipelineStateManager::getShaderSignature(const std::string &name)
{
    auto it = shaderSignatures.find(name);
    if(it != shaderSignatures.end())
        return it->second;
    return nullptr;
}

void PipelineStateManager::addShaderSignature(const std::string &name, const agpu_ref<agpu_shader_signature> &shaderSignature)
{
    shaderSignatures[name] = shaderSignature;
}

agpu_ref<agpu_vertex_layout> PipelineStateManager::getVertexLayout(size_t vertexBufferCount, size_t layoutSize, agpu_vertex_attrib_description *layout)
{
    auto layoutName = std::make_pair(layout, std::make_pair(vertexBufferCount, layoutSize));
    auto it = vertexLayouts.find(layoutName);
    if(it != vertexLayouts.end())
        return it->second;

    agpu_ref<agpu_vertex_layout> vlayout = device->createVertexLayout();
    vlayout->addVertexAttributeBindings(vertexBufferCount, layoutSize, layout);
    vertexLayouts[layoutName] = vlayout;
    return vlayout;
}

agpu_ref<agpu_shader> PipelineStateManager::compileShaderFromFile(const std::string &fileName, agpu_shader_type type)
{
    // Read the source file
    std::string fullName = fileName;
    switch (preferredShaderLanguage)
    {
    case AGPU_SHADER_LANGUAGE_GLSL:
        fullName += ".glsl";
        break;
    case AGPU_SHADER_LANGUAGE_HLSL:
        fullName += ".hlsl";
        break;
    case AGPU_SHADER_LANGUAGE_BINARY:
        fullName += ".cso";
        break;
    case AGPU_SHADER_LANGUAGE_SPIR_V:
        fullName += ".spirv";
        break;
    default:
        break;
    }

    auto source = readWholeFile(fullName);
    if(source.empty())
        return nullptr;

    // Create the shader and compile it.
    agpu_ref<agpu_shader> shader = device->createShader(type);
    shader->setShaderSource(preferredShaderLanguage, source.c_str(), (agpu_string_length)source.size());
    try
    {
        shader->compileShader(nullptr);
    }
    catch(agpu_error &e)
    {
        auto logLength = shader->getCompilationLogLength();
        std::unique_ptr<char[]> logBuffer(new char[logLength+1]);
        shader->getCompilationLog(logLength+1, logBuffer.get());
        printError("Compilation error of '%s':%s\n", fullName.c_str(), logBuffer.get());
        return nullptr;
    }

    return shader;
}

agpu_ref<agpu_shader> PipelineStateManager::getShaderFromFile(const std::string &fileName, agpu_shader_type type)
{
    auto nameType = std::make_pair(fileName, type);
    auto it = shaders.find(nameType);
    if(it != shaders.end())
        return it->second;

    auto shader = compileShaderFromFile(fileName, type);
    if(!shader)
        return shader;

    shaders[nameType] = shader;
    return shader;
}

agpu_ref<agpu_pipeline_state> PipelineStateManager::getState(PipelineStateFactory &factory)
{
    auto internalBuilder = device->createPipelineBuilder();
    if(!internalBuilder)
        agpu_ref<agpu_pipeline_state>();

    PipelineBuilder builder(this, internalBuilder);
	return factory.build(builder);
}

} // End of namespace Loden
