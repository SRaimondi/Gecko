// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "spdlog/spdlog.h"

static void processInput(GLFWwindow *const window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void glfwErrorCallback([[maybe_unused]] const int error,
                              const char *description) {
  spdlog::error("GLFW error: %s", description);
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

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
