#include "Loden/PipelineStateManager.hpp"
#include "Loden/Printing.hpp"
#include "Loden/Engine.hpp"
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

PipelineStateManager::PipelineStateManager(Engine *engine)
	: engine(engine)
{
    device = engine->getAgpuDevice();
    buildParseTables();
    buildPipelineStateParsingActions();
}

PipelineStateManager::~PipelineStateManager()
{
}


void PipelineStateManager::buildParseTables()
{
    // Structure type.
    structureTypeMap["generic"] = StructureType::Generic;
    structureTypeMap["uniform"] = StructureType::UniformState;
    structureTypeMap["uniform-state"] = StructureType::UniformState;
    structureTypeMap["vertex"] = StructureType::Vertex;

    // Structure field type.
    for (int i = 0; i < (int)StructureFieldType::Count; ++i)
    {
        auto &desc = StructureFieldTypeDescription::Descriptions[i];
        structureFieldTypeMap[desc.name] = StructureFieldType(i);
    }

    // Some dictionaries for parsing data.
    shaderBindingTypeNameMap["cbv"] = AGPU_SHADER_BINDING_TYPE_CBV;
    shaderBindingTypeNameMap["srv"] = AGPU_SHADER_BINDING_TYPE_SRV;
    shaderBindingTypeNameMap["sampler"] = AGPU_SHADER_BINDING_TYPE_SAMPLER;
    shaderBindingTypeNameMap["uav"] = AGPU_SHADER_BINDING_TYPE_UAV;

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

    // Blending factor
    blendingFactorNameMap["zero"] = AGPU_BLENDING_ZERO;
    blendingFactorNameMap["one"] = AGPU_BLENDING_ONE;
    blendingFactorNameMap["source-color"] = AGPU_BLENDING_SRC_COLOR;
    blendingFactorNameMap["inverted-source-color"] = AGPU_BLENDING_INVERTED_SRC_COLOR;
    blendingFactorNameMap["source-alpha"] = AGPU_BLENDING_SRC_ALPHA;
    blendingFactorNameMap["inverted-source-alpha"] = AGPU_BLENDING_INVERTED_SRC_ALPHA;
    blendingFactorNameMap["dest-alpha"] = AGPU_BLENDING_DEST_ALPHA;
    blendingFactorNameMap["inverted-dest-alpha"] = AGPU_BLENDING_INVERTED_DEST_ALPHA;
    blendingFactorNameMap["dest-color"] = AGPU_BLENDING_DEST_COLOR;
    blendingFactorNameMap["inverted-dest-color"] = AGPU_BLENDING_INVERTED_DEST_COLOR;
    blendingFactorNameMap["source-alpha-sat"] = AGPU_BLENDING_SRC_ALPHA_SAT;

    // Blending operation
    blendingOperationNameMap["add"] = AGPU_BLENDING_OPERATION_ADD;
    blendingOperationNameMap["subtract"] = AGPU_BLENDING_OPERATION_SUBTRACT;
    blendingOperationNameMap["reverse-subtract"] = AGPU_BLENDING_OPERATION_REVERSE_SUBTRACT;
    blendingOperationNameMap["max"] = AGPU_BLENDING_OPERATION_MAX;
    blendingOperationNameMap["min"] = AGPU_BLENDING_OPERATION_MIN;

    // Compare function
    compareFunctionNameMap["always"] = AGPU_ALWAYS;
    compareFunctionNameMap["never"] = AGPU_NEVER;
    compareFunctionNameMap["less"] = AGPU_LESS;
    compareFunctionNameMap["less-equal"] = AGPU_LESS_EQUAL;
    compareFunctionNameMap["equal"] = AGPU_EQUAL;
    compareFunctionNameMap["not-equal"] = AGPU_NOT_EQUAL;
    compareFunctionNameMap["greater"] = AGPU_GREATER;
    compareFunctionNameMap["greater-equal"] = AGPU_GREATER_EQUAL;

    // Stencil operation
    stencilOperationNameMap["keep"] = AGPU_KEEP;
    stencilOperationNameMap["zero"] = AGPU_ZERO;
    stencilOperationNameMap["replace"] = AGPU_REPLACE;
    stencilOperationNameMap["invert"] = AGPU_INVERT;
    stencilOperationNameMap["increase"] = AGPU_INCREASE;
    stencilOperationNameMap["increase-wrap"] = AGPU_INCREASE_WRAP;
    stencilOperationNameMap["decrease"] = AGPU_DECREASE;
    stencilOperationNameMap["decrease-wrap"] = AGPU_DECREASE_WRAP;

    // Texture formats
    for (int i = 0;; ++i)
    {
        auto format = &TextureFormatDescription::Descriptions[i];
        if (!format->name)
            break;

        textureFormatNameMap[format->name] = format;
    }
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

    pipelineStateParsingActions["render-target-formats"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsArray())
            return false;

        auto count = std::min((int)value.Size(), 16);
        std::vector<agpu_texture_format> formats(count);
        for (int i = 0; i < count; ++i)
        {
            auto formatIt = textureFormatNameMap.find(value.GetString());
            if (formatIt == textureFormatNameMap.end())
            {
                printError("Failed to texture format %s\n", value.GetString());
                return false;
            }

            formats[i] = formatIt->second->format;
        }

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setRenderTargetCount(count);
            for (int i = 0; i < count; ++i)
                builder->setRenderTargetFormat(i, formats[i]);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["depth-stencil-format"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsString())
            return false;

        auto formatIt = textureFormatNameMap.find(value.GetString());
        if (formatIt == textureFormatNameMap.end())
        {
            printError("Failed to texture format %s\n", value.GetString());
            return false;
        }

        auto format = formatIt->second;
        if (!format->hasDepth || !format->hasStencil)
        {
            printError("Texture format %s has no depth or stencil\n", value.GetString());
            return false;
        }

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setDepthStencilFormat(format->format);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["color-mask"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsObject() || !value.HasMember("render-target-mask") ||
            !value.HasMember("red-enabled") || !value.HasMember("green-enabled") || !value.HasMember("blue-enabled") ||
            !value.HasMember("alpha-enabled"))
            return false;

        auto renderTargetMask = value["render-target-mask"].GetInt();
        auto redEnabled = value["red-enabled"].GetBool();
        auto greenEnabled = value["green-enabled"].GetBool();
        auto blueEnabled = value["blue-enabled"].GetBool();
        auto alphaEnabled = value["alpha-enabled"].GetBool();

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setColorMask(renderTargetMask, redEnabled, greenEnabled, blueEnabled, alphaEnabled);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["blend-state"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsArray())
            return false;

        for (auto i = 0; i < value.Size(); ++i)
        {
            auto &state = value[i];
            if (!state.IsObject())
                return false;

            if (!state.IsObject() || !state.HasMember("render-target-mask") ||
                !state.HasMember("enabled") )
                return false;

            auto renderTargetMask = state["render-target-mask"].GetInt();
            auto enabled = state["enabled"].GetBool();
            auto sourceFactor = AGPU_BLENDING_ONE;
            auto destFactor = AGPU_BLENDING_ZERO;
            auto operation = AGPU_BLENDING_OPERATION_ADD;
            auto sourceFactorAlpha = AGPU_BLENDING_ONE;
            auto destFactorAlpha = AGPU_BLENDING_ZERO;
            auto operationAlpha = AGPU_BLENDING_OPERATION_ADD;

            if (enabled)
            {
                if (!state.HasMember("source-factor") || !state.HasMember("dest-factor") || !state.HasMember("operation"))
                    return false;

                sourceFactorAlpha = sourceFactor = blendingFactorNameMap[state["source-factor"].GetString()];
                destFactorAlpha = destFactor = blendingFactorNameMap[state["dest-factor"].GetString()];
                operationAlpha = operation = blendingOperationNameMap[state["operation"].GetString()];

                if (state.HasMember("source-factor-alpha") && state.HasMember("dest-factor-alpha") && state.HasMember("operation-alpha"))
                {
                    sourceFactorAlpha = blendingFactorNameMap[state["source-factor-alpha"].GetString()];
                    destFactorAlpha = blendingFactorNameMap[state["dest-factor-alpha"].GetString()];
                    operationAlpha = blendingOperationNameMap[state["operation-alpha"].GetString()];
                }
            }

            stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
                builder->setBlendState(renderTargetMask, enabled);
                builder->setBlendFunction(renderTargetMask, sourceFactor, destFactor, operation, sourceFactorAlpha, destFactorAlpha, operationAlpha);
                return true;
            });
        }

        return true;
    };

    pipelineStateParsingActions["stencil-state"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsObject() || !value.HasMember("enabled") || !value.HasMember("read-mask") || !value.HasMember("write-mask"))
            return false;

        auto enabled = value["enabled"].GetBool();
        auto readMask = value["read-mask"].GetInt();
        auto writeMask = value["write-mask"].GetInt();

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setStencilState(enabled, writeMask, readMask);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["stencil-operations"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsObject() || !value.HasMember("stencil-fail") || !value.HasMember("depth-fail") || !value.HasMember("stencil-depth-pass") || !value.HasMember("function"))
            return false;

        auto stencilFailOp = stencilOperationNameMap[value["stencil-fail"].GetString()];
        auto depthFailOp = stencilOperationNameMap[value["depth-fail"].GetString()];
        auto stencilDepthPassOp = stencilOperationNameMap[value["stencil-depth-pass"].GetString()];
        auto function = compareFunctionNameMap[value["function"].GetString()];
        
        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setStencilFrontFace(stencilFailOp, depthFailOp, stencilDepthPassOp, function);
            builder->setStencilBackFace(stencilFailOp, depthFailOp, stencilDepthPassOp, function);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["stencil-front-operations"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsObject() || !value.HasMember("stencil-fail") || !value.HasMember("depth-fail") || !value.HasMember("stencil-depth-pass") || !value.HasMember("function"))
            return false;

        auto stencilFailOp = stencilOperationNameMap[value["stencil-fail"].GetString()];
        auto depthFailOp = stencilOperationNameMap[value["depth-fail"].GetString()];
        auto stencilDepthPassOp = stencilOperationNameMap[value["stencil-depth-pass"].GetString()];
        auto function = compareFunctionNameMap[value["function"].GetString()];

        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setStencilFrontFace(stencilFailOp, depthFailOp, stencilDepthPassOp, function);
            return true;
        });

        return true;
    };

    pipelineStateParsingActions["stencil-back-operations"] = [&](PipelineStateTemplate &stateTemplate, rapidjson::Value &value) {
        if (!value.IsObject() || !value.HasMember("stencil-fail") || !value.HasMember("depth-fail") || !value.HasMember("stencil-depth-pass") || !value.HasMember("function"))
            return false;

        auto stencilFailOp = stencilOperationNameMap[value["stencil-fail"].GetString()];
        auto depthFailOp = stencilOperationNameMap[value["depth-fail"].GetString()];
        auto stencilDepthPassOp = stencilOperationNameMap[value["stencil-depth-pass"].GetString()];
        auto function = compareFunctionNameMap[value["function"].GetString()];
        
        stateTemplate.addAction([=](const agpu_pipeline_builder_ref &builder) {
            builder->setStencilBackFace(stencilFailOp, depthFailOp, stencilDepthPassOp, function);
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

    // Load the structures
    {
        auto &structuresFileName = document["structures"];
        assert(structuresFileName.IsString());
        if (!loadStructuresFromFile(joinPath(basePath, structuresFileName.GetString())))
            return false;
    }

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

bool PipelineStateManager::loadStructuresFromFile(const std::string &filename)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(filename, &document))
        return false;

    assert(document.IsObject());
    for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
    {
        assert(it->name.IsString());
        auto name = it->name.GetString();

        auto &structureDescription = it->value;
        assert(structureDescription.IsObject());

        if (!structureDescription.HasMember("type") || !structureDescription["type"].IsString())
        {
            printError("Missing type in structure specification.\n");
            return false;
        }

        if (!structureDescription.HasMember("fields") || !structureDescription["fields"].IsArray())
        {
            printError("Missing fields in structure specification.\n");
            return false;
        }

        auto structure = std::make_shared<Structure> ();

        // Parse the structure type
        {
            auto typeName = structureDescription["type"].GetString();
            auto typeIt = structureTypeMap.find(typeName);
            if (typeIt == structureTypeMap.end())
            {
                printError("Unsupported structure type '%s'.\n", typeName);
                return false;
            }
            structure->type = typeIt->second;
        }

        // Get structure fields
        auto &fields = structureDescription["fields"];

        // Keep track of the structure size and alignment.
        structure->size = 0;
        structure->alignment = 1;

        // Get the fields.
        for (size_t j = 0; j < fields.Size(); ++j)
        {
            auto &fieldValue = fields[j];
            if (!fieldValue.IsObject())
                continue;

            if (!fieldValue.HasMember("type") || !fieldValue.HasMember("name"))
                continue;

            auto &typeValue = fieldValue["type"];
            auto &nameValue = fieldValue["name"];
            if (!nameValue.IsString() || !typeValue.IsString())
                continue;

            auto binding = -1;
            if (fieldValue.HasMember("binding"))
            {
                auto &bindingValue = fieldValue["binding"];
                if(bindingValue.IsInt())
                    binding = bindingValue.GetInt();
            }

            // Parse the field type
            auto typeIt = structureFieldTypeMap.find(typeValue.GetString());
            if (typeIt == structureFieldTypeMap.end())
            {
                printf("Unsupported type '%s' for structure field.", typeValue.GetString());
                return false;
            }

            // Align the attribute offset.
            auto typeId = typeIt->second;
            auto &type = StructureFieldTypeDescription::Descriptions[(int)typeId];
            size_t alignmentMask = type.alignment - 1;
            structure->size = (structure->size + alignmentMask) & ~alignmentMask;
            structure->alignment = std::max(structure->alignment, (size_t)type.alignment);

            // Set the field data.
            StructureField field;
            field.type = typeId;
            field.name = name;
            field.binding = binding;
            field.offset = structure->size;
            structure->size += type.size;
            structure->fields.push_back(field);
        }

        // Align the structure size.
        auto alignmentMask = structure->alignment - 1;
        structure->size = (structure->size + alignmentMask) & ~alignmentMask;

        // Register the structure
        addStructure(name, structure);
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

        
        if (!layoutData.HasMember("buffers"))
        {
            printError("Missing buffers count in vertex layout specification.\n");
            return false;
        }

        auto &bufferArray = layoutData["buffers"];
        if (!bufferArray.IsArray())
        {
            printError("Expected an array with the specification of buffers used by a vertex layout\n");
            return false;
        }

        std::vector<agpu_vertex_attrib_description> layoutAttributes;
        auto bufferCount = bufferArray.Size();
        for (size_t i = 0; i < bufferCount; ++i)
        {
            auto &bufferSpec = bufferArray[i];
            if (!bufferSpec.IsString())
            {
                printError("Expected a structure name to specify a buffer used by a vertex layout.\n");
                return false;
            }

            auto structureName = bufferSpec.GetString();
            auto structure = getStructure(structureName);
            if (!structure)
            {
                printError("Expected unknown structure '%s' required by vertex layout '%s'.\n", structureName, name);
                return false;
            }

            // Add all the attributes of the structure.
            for (auto &field : structure->fields)
            {
                auto &type = StructureFieldTypeDescription::Descriptions[(int)field.type];
                // Set the attribute data.
                agpu_vertex_attrib_description attribute;
                attribute.buffer = (agpu_uint)i;
                attribute.binding = field.binding;
                attribute.type = type.type;
                attribute.components = type.components;
                attribute.rows = type.rows;
                attribute.normalized = type.normalized;
                attribute.divisor = 0; // TODO: Fetch a divisor from some place.
                attribute.offset = field.offset;
                attribute.internal_format = AGPU_TEXTURE_FORMAT_UNKNOWN;
                layoutAttributes.push_back(attribute);
            }
        }

        // Create the vertex layout.
        agpu_vertex_layout_ref vertexLayout = device->createVertexLayout();
        vertexLayout->addVertexAttributeBindings(bufferCount, layoutAttributes.size(), &layoutAttributes[0]);
        addVertexLayout(name, vertexLayout);
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

StructurePtr PipelineStateManager::getStructure(const std::string &name)
{
    auto it = structures.find(name);
    if (it != structures.end())
        return it->second;
    return nullptr;
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

void PipelineStateManager::addStructure(const std::string &name, const StructurePtr &structure)
{
    structures[name] = structure;
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
    catch(agpu_exception &e)
    {
        auto logLength = shader->getCompilationLogLength();
        std::unique_ptr<char[]> logBuffer(new char[logLength+1]);
        shader->getCompilationLog(logLength+1, logBuffer.get());
        printError("[%s]Compilation error of '%s':%s\n", e.what(), fileName.c_str(), logBuffer.get());
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
