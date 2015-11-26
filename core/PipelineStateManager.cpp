#include "Loden/PipelineStateManager.hpp"
#include "Loden/Printing.hpp"
#include "Loden/FileSystem.hpp"
#include <algorithm>
#include <vector>
#include <string.h>

namespace Loden
{

inline const char *mapShaderLanguageIdToName(agpu_shader_language language)
{
    switch (language)
    {
    case AGPU_SHADER_LANGUAGE_GLSL: return "glsl";
    case AGPU_SHADER_LANGUAGE_HLSL: return "hlsl";
    case AGPU_SHADER_LANGUAGE_BINARY: return "binary";
    case AGPU_SHADER_LANGUAGE_SPIR_V: return "spir-v";
    default:
        return NULL;
    }
}

PipelineStateManager::PipelineStateManager(const agpu_device_ref &device)
	: device(device)
{
    buildParseTables();
    buildPipelineStateParsingActions();
}

PipelineStateManager::~PipelineStateManager()
{
}


void PipelineStateManager::buildParseTables()
{
    // Some dictionaries for parsing data.
    shaderBindingTypeNameMap["cbv"] = AGPU_SHADER_BINDING_TYPE_CBV;
    shaderBindingTypeNameMap["srv"] = AGPU_SHADER_BINDING_TYPE_SRV;
    shaderBindingTypeNameMap["sampler"] = AGPU_SHADER_BINDING_TYPE_SAMPLER;
    shaderBindingTypeNameMap["uav"] = AGPU_SHADER_BINDING_TYPE_UAV;

    // Vertex attributes
    vertexLayoutTypeMap["float"] = VertexAttributeType(AGPU_FLOAT, 1, 1, false, 4, 4);
    vertexLayoutTypeMap["vec2"] = VertexAttributeType(AGPU_FLOAT, 2, 1, false, 4, 8);
    vertexLayoutTypeMap["vec3"] = VertexAttributeType(AGPU_FLOAT, 3, 1, false, 4, 12);
    vertexLayoutTypeMap["vec4"] = VertexAttributeType(AGPU_FLOAT, 4, 1, false, 4, 16);

    vertexLayoutTypeMap["int"] = VertexAttributeType(AGPU_INT, 1, 1, false, 4, 4);
    vertexLayoutTypeMap["ivec2"] = VertexAttributeType(AGPU_INT, 2, 1, false, 4, 8);
    vertexLayoutTypeMap["ivec3"] = VertexAttributeType(AGPU_INT, 3, 1, false, 4, 12);
    vertexLayoutTypeMap["ivec4"] = VertexAttributeType(AGPU_INT, 4, 1, false, 4, 16);

    vertexLayoutTypeMap["uint"] = VertexAttributeType(AGPU_UNSIGNED_INT, 1, 1, false, 4, 4);
    vertexLayoutTypeMap["uivec2"] = VertexAttributeType(AGPU_UNSIGNED_INT, 2, 1, false, 4, 8);
    vertexLayoutTypeMap["uivec3"] = VertexAttributeType(AGPU_UNSIGNED_INT, 3, 1, false, 4, 12);
    vertexLayoutTypeMap["uivec4"] = VertexAttributeType(AGPU_UNSIGNED_INT, 4, 1, false, 4, 16);

    vertexLayoutTypeMap["short"] = VertexAttributeType(AGPU_SHORT, 1, 1, false, 2, 2);
    vertexLayoutTypeMap["svec2"] = VertexAttributeType(AGPU_SHORT, 2, 1, false, 4, 4);
    vertexLayoutTypeMap["svec4"] = VertexAttributeType(AGPU_SHORT, 4, 1, false, 4, 8);

    vertexLayoutTypeMap["ushort"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 1, 1, false, 2, 2);
    vertexLayoutTypeMap["usvec2"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 2, 1, false, 4, 4);
    vertexLayoutTypeMap["usvec4"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 4, 1, false, 4, 8);

    vertexLayoutTypeMap["nshort"] = VertexAttributeType(AGPU_SHORT, 1, 1, true, 2, 2);
    vertexLayoutTypeMap["nsvec2"] = VertexAttributeType(AGPU_SHORT, 2, 1, true, 4, 4);
    vertexLayoutTypeMap["nsvec4"] = VertexAttributeType(AGPU_SHORT, 4, 1, true, 4, 8);

    vertexLayoutTypeMap["nushort"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 1, 1, true, 2, 2);
    vertexLayoutTypeMap["nusvec2"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 2, 1, true, 4, 4);
    vertexLayoutTypeMap["nusvec4"] = VertexAttributeType(AGPU_UNSIGNED_SHORT, 4, 1, true, 4, 8);

    vertexLayoutTypeMap["byte"] = VertexAttributeType(AGPU_BYTE, 1, 1, false, 1, 1);
    vertexLayoutTypeMap["bvec2"] = VertexAttributeType(AGPU_BYTE, 2, 1, false, 2, 2);
    vertexLayoutTypeMap["bvec4"] = VertexAttributeType(AGPU_BYTE, 4, 1, false, 4, 4);

    vertexLayoutTypeMap["ubyte"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 1, 1, false, 1, 1);
    vertexLayoutTypeMap["ubvec2"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 2, 1, false, 2, 2);
    vertexLayoutTypeMap["ubvec4"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 4, 1, false, 4, 4);

    vertexLayoutTypeMap["nbyte"] = VertexAttributeType(AGPU_BYTE, 1, 1, true, 1, 1);
    vertexLayoutTypeMap["nbvec2"] = VertexAttributeType(AGPU_BYTE, 2, 1, true, 2, 2);
    vertexLayoutTypeMap["nbvec4"] = VertexAttributeType(AGPU_BYTE, 4, 1, true, 4, 4);

    vertexLayoutTypeMap["nubyte"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 1, 1, true, 1, 1);
    vertexLayoutTypeMap["nubvec2"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 2, 1, true, 2, 2);
    vertexLayoutTypeMap["nubvec4"] = VertexAttributeType(AGPU_UNSIGNED_BYTE, 4, 1, true, 4, 4);

    // Shader type name.
    shaderTypeNameMap["vertex"] = AGPU_VERTEX_SHADER;
    shaderTypeNameMap["fragment"] = AGPU_FRAGMENT_SHADER;
    shaderTypeNameMap["compute"] = AGPU_COMPUTE_SHADER;
    shaderTypeNameMap["geometry"] = AGPU_GEOMETRY_SHADER;
    shaderTypeNameMap["tessellation-control"] = AGPU_TESSELLATION_CONTROL_SHADER;
    shaderTypeNameMap["tessellation-evaluation"] = AGPU_TESSELLATION_EVALUATION_SHADER;

    // Compiled shader language
    {
        auto language = device->getPreferredShaderLanguage();
        auto name = mapShaderLanguageIdToName(language);
        if(name)
            shaderLanguageSearchOrder.push_back(std::make_pair(language, name));
    }

    {
        auto language = device->getPreferredHighLevelShaderLanguage();
        auto name = mapShaderLanguageIdToName(language);
        if (name)
            shaderLanguageSearchOrder.push_back(std::make_pair(language, name));
    }

    // Primitive type name map.
    primitiveTypeNameMap["point"] = AGPU_PRIMITIVE_TYPE_POINT;
    primitiveTypeNameMap["line"] = AGPU_PRIMITIVE_TYPE_LINE;
    primitiveTypeNameMap["triangle"] = AGPU_PRIMITIVE_TYPE_TRIANGLE;
    primitiveTypeNameMap["patch"] = AGPU_PRIMITIVE_TYPE_PATCH;
}

void PipelineStateManager::buildPipelineStateParsingActions()
{
    pipelineStateParsingActions["abstract"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsBool())
            return false;

        stateTemplate.isAbstract = value.GetBool();
        return true;
    };

    pipelineStateParsingActions["inherit-from"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        // Find parent.
        auto parent = getPipelineStateTemplate(value.GetString());
        if(!parent)
            parent = getPipelineStateTemplate(stateTemplate.namePrefix + value.GetString());

        // Ensure the parent was found.
        if (!parent)
        {
            printError("Failed to find pipeline state template '%s'\n", value.GetString());
            return false;
        }

        stateTemplate.parent = parent;
        return true;
    };

    pipelineStateParsingActions["shader-signature"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        auto signature = getShaderSignature(value.GetString());
        if (!signature)
        {
            printError("Failed to find shader signature %s\n", value.GetString());
            return false;
        }

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setShaderSignature(signature.get());
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["vertex-layout"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        auto vertexLayout = getVertexLayout(value.GetString());
        if (!vertexLayout)
        {
            printError("Failed to find vertex layout %s\n", value.GetString());
            return false;
        }

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setVertexLayout(vertexLayout.get());
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["shader"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        auto shaderSet = getShaderSet(value.GetString());
        if (!shaderSet)
        {
            printError("Failed to find vertex layout %s\n", value.GetString());
            return false;
        }

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            for (auto &shader : shaderSet->stages)
                builder->attachShader(shader.get());
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["render-target-count"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsInt())
            return false;

        auto count = value.GetInt();
        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setRenderTargetCount(count);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["primitive-type"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        auto primitiveTypeIt = primitiveTypeNameMap.find(value.GetString());
        if (primitiveTypeIt == primitiveTypeNameMap.end())
        {
            printError("Invalid primitive type '%s'\n", value.GetString());
            return false;
        }

        auto primitiveType = primitiveTypeIt->second;
        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setPrimitiveType(primitiveType);
            return true;
        });

        return true;
    };
}

bool PipelineStateManager::initialize()
{
    return loadStatesFromFile("core-assets/pipeline-states/states.json");
}

bool PipelineStateManager::loadStatesFromFile(const std::string &filename)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    auto basePath = dirname(filename);

    // Load the shader signatures
    {
        auto &shaderSignatureFileName = document["shader-signatures"];
        assert(shaderSignatureFileName.IsString());
        if (!loadShaderSignaturesFromFile(joinPath(basePath, shaderSignatureFileName.GetString())))
            return false;
    }

    // Load the vertex layouts
    {
        auto &vertexLayoutsFileName = document["vertex-layouts"];
        assert(vertexLayoutsFileName.IsString());
        if (!loadVertexLayoutsFromFile(joinPath(basePath, vertexLayoutsFileName.GetString())))
            return false;
    }

    // Load the shaders
    {
        auto &shadersFileName = document["shaders"];
        assert(shadersFileName.IsString());
        if (!loadShadersFromFile(joinPath(basePath, shadersFileName.GetString())))
            return false;
    }

    // Load the pipeline states.
    {
        auto &pipelineStatesFileName = document["pipeline-states"];
        assert(pipelineStatesFileName.IsString());
        if (!loadPipelineStatesFromFile(joinPath(basePath, pipelineStatesFileName.GetString())))
            return false;
    }

    return true;
}

bool PipelineStateManager::loadShaderSignaturesFromFile(const std::string &filename)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    assert(document.IsObject());
    for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
    {
        assert(it->name.IsString());
        auto name = it->name.GetString();
        auto &elements = it->value;
        if (!elements.IsArray())
        {
            printf("Invalid data for shader signature named '%s'.\n", name);
            continue;
        }

        agpu_shader_signature_builder_ref signatureBuilder = device->createShaderSignatureBuilder();
        for (size_t i = 0; i < elements.Size(); ++i)
        {
            auto &element = elements[i];
            if (!element.IsObject())
                continue;

            // Get the type.
            auto &typeValue = element["type"];
            if (!typeValue.IsString())
                continue;
            auto type = typeValue.GetString();

            // Constants are special.
            if (!strcmp(type, "constant"))
            {
                signatureBuilder->addBindingConstant();
                continue;
            }

            // Get the binding type.
            auto &bindingTypeValue = element["binding-type"];
            if (!bindingTypeValue.IsString())
                continue;

            // Parse the binding type.
            auto bindingTypeIt = shaderBindingTypeNameMap.find(bindingTypeValue.GetString());
            if (bindingTypeIt == shaderBindingTypeNameMap.end())
            {
                printError("Invalid shader binding type '%s' in shader signature named '%s'", bindingTypeValue.GetString(), name);
                continue;
            }
            auto bindingType = bindingTypeIt->second;

            // Get the max number of bindings
            auto &maxBindingsValue = element["max-bindings"];
            if (!maxBindingsValue.IsInt())
                continue;
            auto maxBindings = maxBindingsValue.GetInt();

            // Add the signature element according to its type.
            if (!strcmp(type, "bank"))
            {
                // Get the binding point count
                auto &bindingPointValue = element["binding-points"];
                if (!bindingPointValue.IsInt())
                    continue;
                auto bindingPointCount = bindingPointValue.GetInt();

                signatureBuilder->addBindingBank(bindingType, bindingPointCount, maxBindings);
            }
            else if (!strcmp(type, "element"))
            {
                signatureBuilder->addBindingElement(bindingType, maxBindings);
            }
            else
            {
                printError("Unsupported shader signature element of type '%s', used in '%s'\n", type, name);
            }
        }

        // Finish building the shader signature.
        agpu_shader_signature_ref shaderSignature = signatureBuilder->build();
        if (!shaderSignature)
        {
            printError("Failed to create the shader signature named '%s'\n", name);
            return false;
        }

        addShaderSignature(name, shaderSignature);

    }
    return true;
}

bool PipelineStateManager::loadVertexLayoutsFromFile(const std::string &filename)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    assert(document.IsObject());
    for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
    {
        assert(it->name.IsString());
        auto name = it->name.GetString();

        auto &layoutData = it->value;
        assert(layoutData.IsObject());

        if (!layoutData.HasMember("buffer-count") || !layoutData["buffer-count"].IsInt())
        {
            printError("Missing buffer count in vertex layout specification.\n");
            return false;
        }

        auto bufferCount = layoutData["buffer-count"].GetInt();

        std::vector<agpu_vertex_attrib_description> layoutAttributes;
        if (layoutData.HasMember("structures") && layoutData["structures"].IsArray())
        {
            auto &structures = layoutData["structures"];
            for (size_t i = 0; i < structures.Size(); ++i)
            {
                auto &structure = structures[i];
                if (!structure.IsObject() || !structure.HasMember("fields"))
                    continue;

                // Get the buffer index.
                auto buffer = 0;
                if (structure.HasMember("buffer"))
                {
                    auto &bufferValue = structure["buffer"];
                    if(bufferValue.IsInt())
                        buffer = bufferValue.GetInt();
                }

                // Get structure fields
                auto &fields = structure["fields"];
                if (!fields.IsArray())
                    continue;

                // Keep track of the structure size and alignment.
                size_t startPosition = layoutAttributes.size();
                size_t currentSize = 0;
                size_t currentAlignment = 1;

                // Get the fields.
                for (size_t j = 0; j < fields.Size(); ++j)
                {
                    auto &fieldValue = fields[j];
                    if (!fieldValue.IsObject())
                        continue;

                    if (!fieldValue.HasMember("binding") || !fieldValue.HasMember("type"))
                        continue;

                    auto &bindingValue = fieldValue["binding"];
                    auto &typeValue = fieldValue["type"];
                    if (!bindingValue.IsInt() || !typeValue.IsString())
                        continue;

                    auto binding = bindingValue.GetInt();
                    auto typeIt = vertexLayoutTypeMap.find(typeValue.GetString());
                    if (typeIt == vertexLayoutTypeMap.end())
                    {
                        printf("Unsupported type '%s' for vertex attribute", typeValue.GetString());
                        return false;
                    }

                    // Align the attribute offset.
                    auto &type = typeIt->second;
                    auto alignmentMask = type.alignment - 1;
                    currentSize = (currentSize + alignmentMask) & ~alignmentMask;
                    currentAlignment = std::max(currentAlignment, (size_t)type.alignment);

                    // Get the optional divisor parameter
                    int divisor = 0;
                    if (fieldValue.HasMember("divisor") && fieldValue["divisor"].IsInt())
                        divisor = fieldValue["divisor"].GetInt();

                    // Set the attribute data.
                    agpu_vertex_attrib_description attribute;
                    attribute.buffer = buffer;
                    attribute.binding = binding;
                    attribute.type = type.type;
                    attribute.components = type.components;
                    attribute.rows = type.rows;
                    attribute.normalized = type.normalized;
                    attribute.divisor = divisor;
                    attribute.offset = currentSize;
                    currentSize += type.size;
                    layoutAttributes.push_back(attribute);
                }
            }

            // Create the vertex layout.
            agpu_vertex_layout_ref vertexLayout = device->createVertexLayout();
            vertexLayout->addVertexAttributeBindings(bufferCount, layoutAttributes.size(), &layoutAttributes[0]);
            addVertexLayout(name, vertexLayout);
        }
    }
    return true;
}

bool PipelineStateManager::loadShadersFromFile(const std::string &filename, const std::string &namePrefix)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    assert(document.IsObject());
    auto basePath = dirname(filename);

    if (document.HasMember("shaders") && document["shaders"].IsObject())
    {
        auto &shaders = document["shaders"];
        for (auto it = shaders.MemberBegin(); it != shaders.MemberEnd(); ++it)
        {
            // Fetch the shader implementation data.
            auto &shaderRawName = it->name;
            auto &shaderImplementations = it->value;
            if (!shaderImplementations.IsObject())
                continue;

            // Build the full shader name.
            auto shaderName = namePrefix + shaderRawName.GetString();

            // Find a shader implementation.
            bool shaderFound = false;
            for (auto &shaderLanguageTypeName : shaderLanguageSearchOrder)
            {
                auto language = shaderLanguageTypeName.first;
                auto &languageName = shaderLanguageTypeName.second;
                if (!shaderImplementations.HasMember(languageName.c_str()))
                    continue;

                auto &implementation = shaderImplementations[languageName.c_str()];
                if (!implementation.IsObject())
                    continue;

                // Compile the shader stagets
                shaderFound = true;
                ShaderSet shaderSet;
                for (auto stageIt = implementation.MemberBegin(); stageIt != implementation.MemberEnd(); ++stageIt)
                {
                    auto &stageNameValue = stageIt->name;
                    auto &stageValue = stageIt->value;
                    if (!stageValue.IsString())
                        continue;

                    // Get the shader type.
                    auto typeIt = shaderTypeNameMap.find(stageNameValue.GetString());
                    if (typeIt == shaderTypeNameMap.end())
                    {
                        printError("Unsupported shader stage of type '%s' for %s\n", stageNameValue.GetString(), shaderName.c_str());
                        return false;
                    }

                    auto type = typeIt->second;

                    // Create the shader stage filename
                    auto stageFileName = joinPath(basePath, stageValue.GetString());

                    // Compile/load the shader stage.
                    auto stage = compileShaderFromFile(stageFileName, language, type);
                    if (!stage)
                        return false;

                    // Store the shader stage.
                    shaderSet.addStage(stage);
                }

                // Store the shader set.
                addShaderSet(shaderName, shaderSet);
            }

            // Ensure that a shader was found
            if (!shaderFound)
            {
                printError("Failed to find a suitable shader implementation for '%s'\n", shaderName.c_str());
                return false;
            }
        }
    }

    if (document.HasMember("groups") && document["groups"].IsObject())
    {
        auto &groups = document["groups"];
        for (auto it = groups.MemberBegin(); it != groups.MemberEnd(); ++it)
        {
            auto &rawName = it->name;
            auto &value = it->value;
            if (!value.IsString())
                continue;

            std::string groupPrefix = namePrefix + rawName.GetString() + ".";
            std::string groupFileName = joinPath(basePath, value.GetString());
            if (!loadShadersFromFile(groupFileName, groupPrefix))
                return false;
        }
    }

    return true;
}

bool PipelineStateManager::loadPipelineStatesFromFile(const std::string &filename, const std::string &namePrefix)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    assert(document.IsObject());
    auto basePath = dirname(filename);

    // Load the states
    if (document.HasMember("states") && document["states"].IsObject())
    {
        auto &states= document["states"];
        for (auto it = states.MemberBegin(); it != states.MemberEnd(); ++it)
        {
            auto &rawName = it->name;
            auto &rawData = it->value;

            if (!rawData.IsObject())
                continue;

            // Create the state name
            auto stateName = namePrefix + rawName.GetString();

            // Create the state template
            auto stateTemplate = std::make_shared<PipelineStateTemplate> ();
            stateTemplate->namePrefix = namePrefix;

            // Parse the template elements.
            for (auto elementIt = rawData.MemberBegin(); elementIt != rawData.MemberEnd(); ++elementIt)
            {
                auto &elementName = elementIt->name;
                auto &elementValue = elementIt->value;

                auto parseActionIt = pipelineStateParsingActions.find(elementName.GetString());
                if (parseActionIt == pipelineStateParsingActions.end())
                {
                    printError("Unknown pipeline state attribute %s used in %s\n", elementName.GetString(), stateName.c_str());
                    continue;
                }

                // Parse the attribute.
                if (!parseActionIt->second(*stateTemplate.get(), elementValue))
                {
                    printError("Attribute parse error for %s in %s\n", elementName.GetString(), stateName.c_str());
                    return false;
                }
            }

            // Store the pipeline state template.
            addPipelineStateTemplate(stateName, stateTemplate);

            // Instantiate the state template.
            if (!stateTemplate->isAbstract)
            {
                agpu_pipeline_builder_ref builder = device->createPipelineBuilder();
                if (!stateTemplate->instantiateOn(builder))
                {
                    printError("Failed to instantiate pipeline state %s\n", stateName.c_str());
                    return false;
                }

                agpu_pipeline_state_ref state = builder->build();
                if (!state)
                {
                    printError("Failed to build pipeline state %s\n", stateName.c_str());
                    return false;
                }

                addPipelineState(stateName, state);
            }
        }

    }

    // Load the sub-groups
    if (document.HasMember("groups") && document["groups"].IsObject())
    {
        auto &groups = document["groups"];
        for (auto it = groups.MemberBegin(); it != groups.MemberEnd(); ++it)
        {
            auto &rawName = it->name;
            auto &value = it->value;
            if (!value.IsString())
                continue;

            std::string groupPrefix = namePrefix + rawName.GetString() + ".";
            std::string groupFileName = joinPath(basePath, value.GetString());
            if (!loadPipelineStatesFromFile(groupFileName, groupPrefix))
                return false;
        }
    }
    return true;
}

const agpu_device_ref &PipelineStateManager::getDevice() const
{
    return device;
}

agpu_shader_signature_ref PipelineStateManager::getShaderSignature(const std::string &name)
{
    auto it = shaderSignatures.find(name);
    if(it != shaderSignatures.end())
        return it->second;
    return nullptr;
}

ShaderSet *PipelineStateManager::getShaderSet(const std::string &name)
{
    auto it = shaderSets.find(name);
    if (it != shaderSets.end())
        return &it->second;
    return nullptr;
}

void PipelineStateManager::addShaderSignature(const std::string &name, const agpu_shader_signature_ref &shaderSignature)
{
    shaderSignatures[name] = shaderSignature;
}

void PipelineStateManager::addVertexLayout(const std::string &name, const agpu_vertex_layout_ref &vertexLayout)
{
    vertexLayouts[name] = vertexLayout;
}

void PipelineStateManager::addShaderSet(const std::string &name, const ShaderSet &shaderSet)
{
    shaderSets[name] = shaderSet;
}

void PipelineStateManager::addPipelineStateTemplate(const std::string &name, const PipelineStateTemplatePtr &stateTemplate)
{
    pipelineStateTemplates[name] = stateTemplate;
}

void PipelineStateManager::addPipelineState(const std::string &name, const agpu_pipeline_state_ref &state)
{
    pipelineStates[name] = state;
}

agpu_vertex_layout_ref PipelineStateManager::getVertexLayout(const std::string &name)
{
    auto it = vertexLayouts.find(name);
    if(it != vertexLayouts.end())
        return it->second;

    return nullptr;
}

agpu_shader_ref PipelineStateManager::compileShaderFromFile(const std::string &fileName, agpu_shader_language language, agpu_shader_type type)
{
    // Read the source file
    auto source = readWholeFile(fileName);
    if(source.empty())
        return nullptr;

    // Create the shader and compile it.
    agpu_shader_ref shader = device->createShader(type);
    shader->setShaderSource(language, source.c_str(), (agpu_string_length)source.size());
    try
    {
        shader->compileShader(nullptr);
    }
    catch(agpu_error &e)
    {
        auto logLength = shader->getCompilationLogLength();
        std::unique_ptr<char[]> logBuffer(new char[logLength+1]);
        shader->getCompilationLog(logLength+1, logBuffer.get());
        printError("Compilation error of '%s':%s\n", fileName.c_str(), logBuffer.get());
        return nullptr;
    }

    return shader;
}

agpu_pipeline_state_ref PipelineStateManager::getPipelineState(const std::string &name)
{
    auto it = pipelineStates.find(name);
    if (it != pipelineStates.end())
        return it->second;
    return nullptr;
}

PipelineStateTemplatePtr PipelineStateManager::getPipelineStateTemplate(const std::string &name)
{
    auto it = pipelineStateTemplates.find(name);
    if (it != pipelineStateTemplates.end())
        return it->second;
    return nullptr;
}

} // End of namespace Loden
