#pragma once

#include "shader.hpp"

namespace Gecko {
namespace Internal {

template <typename Shader, typename... Shaders>
void attachAll(GLuint program_id, Shader &&shader, Shaders &&... shaders) {
  glAttachShader(program_id, shader.getID());
  if constexpr (sizeof...(shaders) > 0) {
    attachAll(program_id, std::forward<Shaders>(shaders)...);
  }
}

} // namespace Internal

class GLSLProgram {
public:
  // Create from shaders
  template <typename... Shaders>
  explicit GLSLProgram(Shaders &&... shaders) : _program_id{glCreateProgram()} {
    static_assert(std::conjunction_v<
                      std::is_same_v<std::decay_t<Shaders>, GLSLShader>...>,
                  "GLSLProgram constructor used with invalid types");
    Internal::attachAll(_program_id, std::forward<Shaders>(shaders)...);
    link();
  }

  ~GLSLProgram();

  // Not copyable or assignable
  GLSLProgram(const GLSLProgram &) = delete;
  GLSLProgram &operator=(const GLSLProgram &) = delete;

private:
  GLuint _program_id;

  void link() const;
};

} // namespace Gecko