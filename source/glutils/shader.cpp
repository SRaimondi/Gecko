#include "glutils/shader.hpp"

#include <stdexcept>
#include <unordered_map>

namespace Gecko {

ShaderType GLSLShader::extensionToShaderType(const std::string &filename) {
  const static std::unordered_map<std::string, ShaderType> extension_map{
      // Vertex shader
      {".vs", ShaderType::Vertex},
      {".vert", ShaderType::Vertex},
      {".vert.glsl", ShaderType::Vertex},
      // Fragment shader
      {".fs", ShaderType::Fragment},
      {".frag", ShaderType::Fragment},
      {".frag.glsl", ShaderType::Fragment},
      // Tess control
      {".tcs", ShaderType::TessControl},
      {".tcs.glsl", ShaderType::TessControl},
      // Tess evaluation
      {".tes", ShaderType::TessEvaluation},
      {".tes.glsl", ShaderType::TessEvaluation},
      // Geometry shader
      {".gs", ShaderType::Geometry},
      {".geom", ShaderType::Geometry},
      {".geom.glsl", ShaderType::Geometry},
      // Compute
      {".cs", ShaderType::Compute},
      {".cs.glsl", ShaderType::Compute}};

  const auto extension_start{filename.find_first_of('.')};
  if (extension_start == std::string_view::npos) {
    throw std::runtime_error{"Could not find shader file extension"};
  }

  const auto shader_type{extension_map.find(filename.substr(extension_start))};
  if (shader_type == extension_map.end()) {
    throw std::runtime_error{"Could not determine shader type from extension"};
  }

  return shader_type->second;
}

} // namespace Gecko