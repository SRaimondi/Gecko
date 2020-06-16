// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "glutils/utils.hpp"
#include "glutils/program.hpp"
#include "camera/orbit_camera.hpp"
#include "scalar_field/scalar_field.hpp"

#include "spdlog/spdlog.h"

#include <array>

static void glfwErrorCallback([[maybe_unused]] const int error,
                              const char *description) {
  spdlog::error("GLFW error: {}", description);
}

static void glfwKeyCallback(GLFWwindow *window, const int key,
                            [[maybe_unused]] const int scancode,
                            const int action, [[maybe_unused]] const int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static Gecko::OrbitCamera camera;
static glm::vec2 previous_mouse_position;
static int mouse_is_down{0};

static void glfwMouseButtonCallback(GLFWwindow *window, const int button,
                                    const int action,
                                    [[maybe_unused]] const int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      mouse_is_down = 1;
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      previous_mouse_position.x = static_cast<float>(x);
      previous_mouse_position.y = static_cast<float>(y);
    } else if (action == GLFW_RELEASE) {
      mouse_is_down = 0;
    }
  }
}

static void glfwMouseCallback([[maybe_unused]] GLFWwindow *window,
                              const double xpos, const double ypos) {
  if (mouse_is_down) {
    const float x_offset{static_cast<float>(xpos) - previous_mouse_position.x};
    const float y_offset{static_cast<float>(ypos) - previous_mouse_position.y};
    previous_mouse_position.x = static_cast<float>(xpos);
    previous_mouse_position.y = static_cast<float>(ypos);
    constexpr static float CAMERA_SENSITIVITY{0.01f};
    camera.rotateVertical(CAMERA_SENSITIVITY * x_offset);
    camera.rotateHorizontal(-CAMERA_SENSITIVITY * y_offset);
  }
}

static void glfwScrollCallback([[maybe_unused]] GLFWwindow *window,
                               [[maybe_unused]] const double xoffset,
                               const double yoffset) {
  constexpr static float SCROLL_SENSITIVITY{0.1f};
  camera.changeRadius(SCROLL_SENSITIVITY * static_cast<float>(-yoffset));
}

int main() {
  try {
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
    glfwSetWindowSizeLimits(window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);
    glfwSetCursorPosCallback(window, glfwMouseCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);

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
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                            nullptr, GL_TRUE);
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
                           GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                           "Debugging enabled");
    }
#endif

    // Create example field
    Gecko::ScalarField<float> field{
        glm::vec3{-1.f}, glm::vec3{1.f}, 256, 256, 256, 0.f};

    {
      constexpr static std::array<glm::vec3, 4> exp_centers{
          glm::vec3{-0.6f, -0.5f, -1.f}, glm::vec3{0.3f, 0.5f, 0.3f},
          glm::vec3{0.8f, 0.8f, -0.1f}, glm::vec3{-0.2f, -0.3f, 0.7f}};
      constexpr static std::array<float, 4> exp_max{3.f, 1.f, 2.f, 5.f};
      constexpr static std::array<float, 4> exp_c{0.2f, 0.3f, 0.1f, 0.6f};

      const glm::vec3 voxel_size{field.computeVoxelSize()};
      for (int k{0}; k != field.zSize(); ++k) {
        const float z_pos{field.min().z + static_cast<float>(k) * voxel_size.z};
        for (int j{0}; j != field.ySize(); ++j) {
          const float y_pos{field.min().y +
                            static_cast<float>(j) * voxel_size.y};
          for (int i{0}; i != field.xSize(); ++i) {
            const float x_pos{field.min().x +
                              static_cast<float>(i) * voxel_size.x};
            const auto gaussian = [](const float x, const float a,
                                     const float b, const float c) -> float {
              const float t{x - b};
              return a * std::exp(-t * t / (2.f * c * c));
            };
            float value{0.f};
            const glm::vec3 p{x_pos, y_pos, z_pos};
            for (std::size_t g_i{0}; g_i != exp_centers.size(); ++g_i) {
              value =
                  std::max(value, gaussian(glm::length(exp_centers[g_i] - p),
                                           exp_max[g_i], 0.f, exp_c[g_i]));
            }
            field(i, j, k) = value;
          }
        }
      }
    }

    // Create program
    const Gecko::GLSLProgram base_program{
        Gecko::GLSLShader::createFromFile("../shaders/base.vert"),
        Gecko::GLSLShader::createFromFile("../shaders/base.frag")};
    base_program.validate();

    // Create triangle
    GLuint vao;
    glGenVertexArrays(1, &vao);
    std::array<GLuint, 2> vbos{0u};
    glGenBuffers(2, vbos.data());
    {
      // clang-format off
      const std::array<float, 9> vertices{-0.5f, -0.5f, 0.f,
                                          0.5f, -0.5f, 0.f,
                                          0.f, 0.5f, 0.f};
      const std::array<float, 9> colors{1.f, 0.f, 0.f,
                                        0.f, 1.f, 0.f,
                                        0.f, 0.f, 1.f};
      // clang-format on

      // First bind vao array
      glBindVertexArray(vao);

      // Now bind vbo and submit data
      glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            nullptr);
      glEnableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), colors.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            nullptr);
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
      glClearColor(0.1f, 0.1f, 0.1f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);

      base_program.use();
      base_program.setMat4("model_matrix", glm::identity<glm::mat4>());
      base_program.setMat4("view_matrix", camera.getViewMatrix());
      // Update perspective matrix
      int framebuffer_width, framebuffer_height;
      glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
      glViewport(0, 0, framebuffer_width, framebuffer_height);
      base_program.setMat4(
          "projection_matrix",
          glm::perspectiveFov(
              glm::radians(60.f), static_cast<float>(framebuffer_width),
              static_cast<float>(framebuffer_height), 0.1f, 100.f));

      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(2, vbos.data());

    glfwDestroyWindow(window);
    glfwTerminate();

  } catch (const std::exception &ex) {
    spdlog::error(ex.what());
  }

  return 0;
}
