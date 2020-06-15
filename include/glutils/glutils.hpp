#pragma once

#include "glad/glad.h"

namespace Gecko::Utils {

void APIENTRY GLDebugCallback(const GLenum source,
                              const GLenum type, const GLuint id,
                              const GLenum severity, const GLsizei length,
                              const GLchar* message, const void* param) noexcept;

}