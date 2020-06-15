#pragma once

#include "glad/glad.h"

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
  ~GLSLShader();

  GLSLShader(const GLSLShader &) = delete;
  GLSLShader &operator=(const GLSLShader &) = delete;

  // Factory functions
  [[nodiscard]] static GLSLShader createFromFile(const std::string &filename);

  [[nodiscard]] ShaderType getType() const noexcept { return _type; }
  [[nodiscard]] GLuint getID() const noexcept { return _shader_id; }

private:
  GLuint _shader_id;
  const ShaderType _type;

  // Create shader from source and type
  GLSLShader(const std::string& filename, const std::string &source, ShaderType type);

  // Get the shader type from the given input file name
  [[nodiscard]] static ShaderType
  extensionToShaderType(const std::string &filename);
};

} // namespace Gecko