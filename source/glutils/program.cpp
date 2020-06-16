#include "glutils/program.hpp"

#include "fmt/format.h"

#include <stdexcept>

namespace Gecko {

void GLSLProgram::validate() const {
  glValidateProgram(_program_id);
  GLint validation_value{0};
  glGetProgramiv(_program_id, GL_VALIDATE_STATUS, &validation_value);
  if (validation_value == GL_FALSE) {
    throw std::runtime_error{fmt::format(
        "Error reported during program validation: {}", getProgramLog())};
  }
}

void GLSLProgram::link() const {
  glLinkProgram(_program_id);
  // Check for errors
  GLint link_result{0};
  glGetProgramiv(_program_id, GL_LINK_STATUS, &link_result);
  if (link_result == GL_FALSE) {
    const auto log{getProgramLog()};
    glDeleteProgram(_program_id);
    throw std::runtime_error{
        fmt::format("Error during program linking: {}", log)};
  }
}

std::string GLSLProgram::getProgramLog() const {
  GLsizei log_length{0};
  glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &log_length);
  std::string log(static_cast<std::string::size_type>(log_length), ' ');
  GLint written{0};
  glGetProgramInfoLog(_program_id, log_length, &written, log.data());
  return log;
}

GLint GLSLProgram::getUniformLocation(const std::string &uniform_name) const {
  const auto it{_uniforms_map.find(uniform_name)};
  if (it == _uniforms_map.end()) {
    const GLint uniform_location{
        glGetUniformLocation(_program_id, uniform_name.c_str())};
    if (uniform_location == -1) {
      throw std::runtime_error{fmt::format(
          "Could not determine location for uniform {}", uniform_name)};
    }
    return uniform_location;
  }
  return it->second;
}

} // namespace Gecko
