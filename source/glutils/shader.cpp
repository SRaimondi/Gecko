#include "glutils/shader.hpp"

#include "fmt/format.h"

#include <array>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace Gecko {

GLSLShader::~GLSLShader() { glDeleteShader(_shader_id); }

GLSLShader GLSLShader::createFromFile(const std::string &filename) {
  std::ifstream shader_file{filename};
  if (!shader_file.is_open()) {
    throw std::runtime_error{
        fmt::format("Could not open file {} in shader creation", filename)};
  }
  const std::string file_source{(std::istreambuf_iterator<char>{shader_file}),
                                std::istreambuf_iterator<char>{}};
  return {filename, file_source, extensionToShaderType(filename)};
}

GLSLShader::GLSLShader(const std::string &filename, const std::string &source,
                       const ShaderType type)
    : _shader_id{glCreateShader(static_cast<GLenum>(type))}, _type{type} {
  const auto source_ptr{reinterpret_cast<const GLchar *>(source.c_str())};
  glShaderSource(_shader_id, 1, &source_ptr, nullptr);
  glCompileShader(_shader_id);
  // Check compile status
  GLint compile_result{0};
  glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &compile_result);
  if (compile_result == GL_FALSE) {
    GLint compile_log_length{0};
    glGetShaderiv(_shader_id, GL_INFO_LOG_LENGTH, &compile_log_length);
    std::string log(static_cast<std::string::size_type>(compile_log_length),
                    ' ');
    GLint written{0};
    glGetShaderInfoLog(_shader_id, compile_log_length, &written, log.data());
    glDeleteShader(_shader_id);
    throw std::runtime_error{
        fmt::format("Error in shader {} compilation: {}", filename, log)};
  }
}

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

  const static std::array<std::string, 2> to_remove_patterns{"..", "./"};
  std::string filename_copy{filename};
  std::for_each(to_remove_patterns.begin(), to_remove_patterns.end(),
                [&filename_copy](const std::string &pattern) {
                  auto pos{std::string ::npos};
                  while ((pos = filename_copy.find(pattern)) !=
                         std::string::npos) {
                    filename_copy.erase(pos, pattern.length());
                  }
                });

  const auto extension_start{filename_copy.find_first_of('.')};
  if (extension_start == std::string_view::npos) {
    throw std::runtime_error{"Could not find shader file extension"};
  }

  const auto shader_type{
      extension_map.find(filename_copy.substr(extension_start))};
  if (shader_type == extension_map.end()) {
    throw std::runtime_error{"Could not determine shader type from extension"};
  }

  return shader_type->second;
}

} // namespace Gecko