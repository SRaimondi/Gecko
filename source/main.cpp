// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "glutils/utils.hpp"

#include "spdlog/spdlog.h"

static void processInput(GLFWwindow *const window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void glfwErrorCallback([[maybe_unused]] const int error,
                              const char *description) {
  spdlog::error("GLFW error: {}", description);
}

static void framebufferSizeCallback([[maybe_unused]] GLFWwindow *window,
                                    const GLsizei width, const GLsizei height) {
  glViewport(0, 0, width, height);
}

int main() {
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    spdlog::error("Could not initialize GLFW");
    return 1;
  }

#if defined(__APPLE__)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (GL_ARB_debug_output) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  }

  // Create GLFW window
  GLFWwindow *window{glfwCreateWindow(800, 600, "Gecko", nullptr, nullptr)};
  if (window == nullptr) {
    spdlog::error("Could not create GLFW window");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  // Initialize GLAD
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    spdlog::error("Failed to initialize GLAD");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

#if !defined(__APPLE__)
  if (GL_ARB_debug_output) {
    glDebugMessageCallback(Gecko::Utils::GLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
                         GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                         "Debugging enabled");
  }
#endif

  // Main render loop
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
