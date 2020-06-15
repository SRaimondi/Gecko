#include "glutils/program.hpp"

#include "fmt/format.h"

#include <stdexcept>

namespace Gecko {

GLSLProgram::~GLSLProgram() { glDeleteProgram(_program_id); }

void GLSLProgram::link() const {
  glLinkProgram(_program_id);
  // Check for errors
  GLint link_result{0};
  glGetProgramiv(_program_id, GL_LINK_STATUS, &link_result);
  if (link_result == GL_FALSE) {
    GLsizei link_log_length{0};
    glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &link_log_length);
    std::string log(static_cast<std::string::size_type>(link_log_length), ' ');
    GLint written{0};
    glGetProgramInfoLog(_program_id, link_log_length, &written, log.data());
    glDeleteProgram(_program_id);
    throw std::runtime_error{
        fmt::format("Error during program linking: {}", log)};
  }
}

} // namespace Gecko