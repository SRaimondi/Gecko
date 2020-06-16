#pragma once

#include "glad/glad.h"

namespace Gecko::Utils {

void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message,
                              const void *param) noexcept;

}
