#include "glutils/utils.hpp"
#include "spdlog/spdlog.h"

#include <string>

namespace Gecko::Utils {

void APIENTRY GLDebugCallback(const GLenum source, const GLenum type,
                              const GLuint id, const GLenum severity,
                              [[maybe_unused]] const GLsizei length,
                              const GLchar *message,
                              [[maybe_unused]] const void *param) noexcept {
  // Generate source string
  const std::string source_str{[source]() -> std::string {
    switch (source) {
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
      return "Window System";
    }
    case GL_DEBUG_SOURCE_APPLICATION: {
      return "Application";
    }
    case GL_DEBUG_SOURCE_API: {
      return "API";
    }
    case GL_DEBUG_SOURCE_SHADER_COMPILER: {
      return "Shader compiler";
    }
    case GL_DEBUG_SOURCE_THIRD_PARTY: {
      return "3rd party";
    }
    case GL_DEBUG_SOURCE_OTHER: {
      return "Other";
    }
    default: {
      return "Unknown";
    }
    }
  }()};

  // Generate type string
  const std::string type_str{[type]() -> std::string {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: {
      return "Error";
    }
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
      return "Deprecated";
    }
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
      return "Undefined";
    }
    case GL_DEBUG_TYPE_PORTABILITY: {
      return "Portability";
    }
    case GL_DEBUG_TYPE_PERFORMANCE: {
      return "Performance";
    }
    case GL_DEBUG_TYPE_MARKER: {
      return "Marker";
    }
    case GL_DEBUG_TYPE_PUSH_GROUP: {
      return "Push group";
    }
    case GL_DEBUG_TYPE_POP_GROUP: {
      return "Pop group";
    }
    case GL_DEBUG_TYPE_OTHER: {
      return "Other";
    }
    default: {
      return "Unknown";
    }
    }
  }()};

  // Generate severity string
  const std::string severity_str{[severity]() -> std::string {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: {
      return "High";
    }
    case GL_DEBUG_SEVERITY_MEDIUM: {
      return "Medium";
    }
    case GL_DEBUG_SEVERITY_LOW: {
      return "Low";
    }
    case GL_DEBUG_SEVERITY_NOTIFICATION: {
      return "Notification";
    }
    default: {
      return "Unknown";
    }
    }
  }()};

  spdlog::info("'{}' {} [{}](ID: {}): {}", source_str, type_str, severity_str,
               id, message);
}

} // namespace Gecko::Utils
