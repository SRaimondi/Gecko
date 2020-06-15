#pragma once

#include "shader.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <unordered_map>

namespace Gecko {

class GLSLProgram {
public:
  template <typename... Shaders>
  explicit GLSLProgram(Shaders &&... shaders) : _program_id{glCreateProgram()} {
    static_assert(
        std::conjunction_v<std::is_same<std::decay_t<Shaders>, GLSLShader>...>,
        "GLSLProgram constructor used with invalid types");
    ((glAttachShader(_program_id, shaders.getID())), ...);
    link();
  }

  ~GLSLProgram() { glDeleteProgram(_program_id); }

  // Not copyable or assignable
  GLSLProgram(const GLSLProgram &) = delete;
  GLSLProgram &operator=(const GLSLProgram &) = delete;

  void validate() const;
  void use() const { glUseProgram(_program_id); }

  [[nodiscard]] GLuint getID() const noexcept { return _program_id; }

  // Uniform setters
  void setInt(const std::string &name, const int value) const {
    glUniform1i(getUniformLocation(name), value);
  }

  void setFloat(const std::string &name, const float value) const {
    glUniform1f(getUniformLocation(name), value);
  }

  void setVec2(const std::string &name, const glm::vec2 &v) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(v));
  }

  void setVec3(const std::string &name, const glm::vec3 &v) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(v));
  }

  void setVec4(const std::string &name, const glm::vec4 &v) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(v));
  }

  void setMat2(const std::string &name, const glm::mat2 &m,
               const GLboolean transpose = GL_FALSE) const {
    glUniformMatrix2fv(getUniformLocation(name), 1, transpose,
                       glm::value_ptr(m));
  }

  void setMat3(const std::string &name, const glm::mat3 &m,
               const GLboolean transpose = GL_FALSE) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, transpose,
                       glm::value_ptr(m));
  }

  void setMat4(const std::string &name, const glm::mat4 &m,
               const GLboolean transpose = GL_FALSE) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, transpose,
                       glm::value_ptr(m));
  }

private:
  GLuint _program_id;
  mutable std::unordered_map<std::string, GLint> _uniforms_map;

  void link() const;

  [[nodiscard]] GLint getUniformLocation(const std::string &uniform_name) const;

  [[nodiscard]] std::string getProgramLog() const;
};

} // namespace Gecko