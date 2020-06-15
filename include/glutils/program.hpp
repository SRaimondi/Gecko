#pragma once

#include "shader.hpp"

namespace Gecko {

class GLSLProgram {
public:
  // Create from shaders
  template <typename... Shaders>
  explicit GLSLProgram(Shaders &&... shaders) : _program_id{glCreateProgram()} {
    static_assert(
        std::conjunction_v<std::is_same<std::decay_t<Shaders>, GLSLShader>...>,
        "GLSLProgram constructor used with invalid types");
    ((glAttachShader(_program_id, shaders.getID())), ...);
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