#pragma once

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>

namespace Gecko {

enum class ShaderType : GLenum {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
  TessControl = GL_TESS_CONTROL_SHADER,
  TessEvaluation = GL_TESS_EVALUATION_SHADER,
  Geometry = GL_GEOMETRY_SHADER,
  Compute = GL_COMPUTE_SHADER
};

class GLSLShader {
public:
  // Not copyable or assignable
  GLSLShader(const GLSLShader &) = delete;
  GLSLShader &operator=(const GLSLShader &) = delete;

private:
  GLuint shader_id;

  static ShaderType extensionToShaderType(const std::string &filename);
};

} // namespace Gecko