#include "shader_core.h"
#include "core/gpu/VK/vk_debug.h"
#include "core/debug.h"

#include "core/configuration/build_generation.h"

#include "yaml-cpp/yaml.h"

#include <fstream>

struct CompilationUnit{
  std::string name;
  VkShaderStageFlagBits stage;
};

struct ShaderEntry{
};


#define YamlNodeOrError(_node) if(!_node.IsDefined()){ssf_runtime_error()};

const char* kPixelShaderBinaryExt = ".pixel.shader";
const char* kVertexShaderBinaryExt = ".vert.shader";

static ShaderStageType MapShaderStage(const std::string& key){
  if(key == "VERTEX"){return ShaderStageVertex;}
  if(key == "PIXEL"){return ShaderStagePixel;}
  return ShaderStageNone;
}

static const std::unordered_map<std::string_view, VkFormat> FormatMap = []{
  std::unordered_map<std::string_view, VkFormat> map;
  map["R32G32B32_SFLOAT"] = VK_FORMAT_R32G32B32_SFLOAT;
  map["R32G32_SFLOAT"] = VK_FORMAT_R32G32_SFLOAT;
  map["R32_SFLOAT"] = VK_FORMAT_R32_SFLOAT;
  return map;
}();

static const std::unordered_map<std::string_view, VkShaderStageFlagBits> ShaderMap = []{
  std::unordered_map<std::string_view, VkShaderStageFlagBits> map;
  map["VERTEX"] = VK_SHADER_STAGE_VERTEX_BIT;
  map["PIXEL"] = VK_SHADER_STAGE_FRAGMENT_BIT;
  return map;
}();

//TODO: switch to in runtime compilation and cacheing.
static VkShaderModule CompilerShader(const CompilationUnit& unit){
  return VK_NULL_HANDLE;
}


static VkShaderModule CompileShaderSource(VkDevice device, VkAllocationCallbacks* allocator, const char* pFile){
  std::ifstream stream{};
  VkShaderModule ret = VK_NULL_HANDLE;

  stream.open(pFile, std::ios::ate | std::ios::binary);

  if(!stream.is_open()){
    ssf_runtime_error();
  }

  size_t bytes = stream.tellg();
  stream.seekg(0);
  void* pCode = malloc(bytes);
  stream.read(static_cast<char*>(pCode), bytes);
  stream.close();
  
  VkShaderModuleCreateInfo cShaderModule{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, bytes,
  static_cast<uint32_t*>(pCode)};

  vkcall(vkCreateShaderModule(device, &cShaderModule, allocator, &ret))
  free(pCode);
  return ret;
}

static int CompileMetaFile(VkDevice device, VkAllocationCallbacks* allocator, const char* pFilePath, ShaderContainer* pContainer){
  YAML::Node root = YAML::LoadFile(pFilePath);

  // //shaders
  CompilationUnit unit;
  VkShaderModule module;
  YAML::Node shaders = root["shaders"];

  VkPipelineShaderStageCreateInfo cShader{};
  cShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  cShader.flags = 0;
  cShader.pSpecializationInfo = nullptr;

  for(auto shader : shaders){
    VkShaderStageFlagBits stage = ShaderMap.find(shader["stage"].as<std::string>())->second;

    std::string name = shader["name"].as<std::string>();
    switch (stage){
      case VK_SHADER_STAGE_VERTEX_BIT:
        name.append(kVertexShaderBinaryExt);
        break;
      case VK_SHADER_STAGE_FRAGMENT_BIT:
        name.append(kPixelShaderBinaryExt);
        break;
      default:
        ssf_runtime_error();
    }

    std::string shaderOut(_SSF_GENERATED_SHADER_FOLDER);
    shaderOut.append(name);
    module =  CompileShaderSource(device, allocator, shaderOut.c_str());

    cShader.stage = stage;
    cShader.module = module;
    cShader.pName = "main";
    pContainer->resources.shaders.push_back(cShader);
  }

  //input
  YAML::Node inputNode = root["input"];
  YamlNodeOrError(inputNode);
  VkVertexInputBindingDescription cInputBinding{};

  for(auto entry : inputNode ){
    VkVertexInputBindingDescription binding{
      entry["slot"].as<uint32_t>(),
      entry["stride"].as<uint32_t>(),
      entry["instance"].as<bool>() == true ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX 
    };
    pContainer->resources.inputs.push_back(binding);
  }

  //vertex attributes
  YAML::Node attribNode = root["vertex-attributes"];
  for(auto entry : attribNode){
    VkVertexInputAttributeDescription attribute{
      entry["location"].as<uint32_t>(),
      entry["binding"].as<uint32_t>(),
      FormatMap.find(entry["format"].as<std::string>())->second,
      entry["offset"].as<uint32_t>(),
    };

    pContainer->resources.attributes.push_back(attribute);
  }

  // push
  YAML::Node pushNode = root["push-constants"];
  for(auto entry : pushNode){
    VkPushConstantRange range{
      ShaderMap.find(entry["stage"].as<std::string>())->second,
      entry["offset"].as<uint32_t>(),
      entry["size"].as<uint32_t>(),
    };

    pContainer->resources.pushRanges.push_back(range);
  }

  // descriptors
  // YAML::Node descriptorNode = root["descriptors"];
  // for(auto set : descriptorNode){
  //   ShaderDescriptorSetLayout descriptorLayout{};
  //   YAML::Node bindingNode = set["binding"];
  //   
  //
  //   for(auto binding : bindingNode){
  //     ShaderDescriptorBinding bindingLayout{};
  //     binding["slot"].as<uint8_t>();
  //     binding["type"].as<std::string>();
  //     binding["stage"].as<std::string>();
  //     descriptorLayout.bindings.push_back(bindingLayout);
  //   }
  //
  //   pContainer->resources.sets.push_back(descriptorLayout);
  // }
  //

  return 0;
}



void ShaderFreeContainer(VkDevice device, VkAllocationCallbacks* allocator, ShaderContainer* pContainer){
  for(auto shader : pContainer->resources.shaders){
    vkDestroyShaderModule(device, shader.module, allocator);
  }

  pContainer->handle = nullptr;
};

ShaderContainer  BuildShaderFromMetaFile(VkDevice device, VkAllocationCallbacks* allocator, const char* pMetaFile){
  ShaderContainer ret;
  CompileMetaFile(device, allocator, pMetaFile, &ret);

  
  return ret;
}
