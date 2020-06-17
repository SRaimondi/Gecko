#if defined(WIN32)
#define NOMINMAX
#endif

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

static Gecko::OrbitCamera camera{glm::vec3{0.f, 0.f, 10.f},
                                 glm::zero<glm::vec3>()};
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
    if (GL_ARB_debug_output) {
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    }
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Print message if debugging is enabled
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

      for (int k{0}; k != field.zSize(); ++k) {
        const float z_pos{field.min().z +
                          static_cast<float>(k) * field.getVoxelSize().z};
        for (int j{0}; j != field.ySize(); ++j) {
          const float y_pos{field.min().y +
                            static_cast<float>(j) * field.getVoxelSize().y};
          for (int i{0}; i != field.xSize(); ++i) {
            const float x_pos{field.min().x +
                              static_cast<float>(i) * field.getVoxelSize().x};
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

    // Copy data to OpenGL texture
    GLuint volume_texture;
    glGenTextures(1, &volume_texture);
    glBindTexture(GL_TEXTURE_3D, volume_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, field.xSize(), field.ySize(),
                 field.zSize(), 0, GL_RED, GL_FLOAT,
                 reinterpret_cast<void *>(field.data()));

    // Create program
    const Gecko::GLSLProgram base_program{
        Gecko::GLSLShader::createFromFile("../shaders/base.vert"),
        Gecko::GLSLShader::createFromFile("../shaders/base.frag")};

    // Create triangle
    GLuint vao;
    glGenVertexArrays(1, &vao);
    static constexpr std::size_t VBO_INDEX{0};
    static constexpr std::size_t EBO_INDEX{1};
    std::array<GLuint, 2> buffers{0};
    glGenBuffers(buffers.size(), buffers.data());

    const std::array<glm::vec3, 48> cube_data{
        // Right face
        glm::vec3{1.f, -1.f, -1.f}, glm::vec3{1.f, 0.f, 0.f},
        glm::vec3{1.f, 1.f, -1.f}, glm::vec3{1.f, 0.f, 0.f},
        glm::vec3{1.f, 1.f, 1.f}, glm::vec3{1.f, 0.f, 0.f},
        glm::vec3{1.f, -1.f, 1.f}, glm::vec3{1.f, 0.f, 0.f},
        // Left face
        glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{-1.f, 0.f, 0.f},
        glm::vec3{-1.f, -1.f, 1.f}, glm::vec3{-1.f, 0.f, 0.f},
        glm::vec3{-1.f, 1.f, 1.f}, glm::vec3{-1.f, 0.f, 0.f},
        glm::vec3{-1.f, 1.f, -1.f}, glm::vec3{-1.f, 0.f, 0.f},
        // Front face
        glm::vec3{-1.f, -1.f, 1.f}, glm::vec3{0.f, 0.f, 1.f},
        glm::vec3{1.f, -1.f, 1.f}, glm::vec3{0.f, 0.f, 1.f},
        glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.f, 0.f, 1.f},
        glm::vec3{-1.f, 1.f, 1.f}, glm::vec3{0.f, 0.f, 1.f},
        // Back face
        glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{0.f, 0.f, -1.f},
        glm::vec3{1.f, -1.f, -1.f}, glm::vec3{0.f, 0.f, -1.f},
        glm::vec3{1.f, 1.f, -1.f}, glm::vec3{0.f, 0.f, -1.f},
        glm::vec3{-1.f, 1.f, -1.f}, glm::vec3{0.f, 0.f, -1.f},
        // Top face
        glm::vec3{-1.f, 1.f, 1.f}, glm::vec3{0.f, 1.f, 0.f},
        glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.f, 1.f, 0.f},
        glm::vec3{1.f, 1.f, -1.f}, glm::vec3{0.f, 1.f, 0.f},
        glm::vec3{-1.f, 1.f, -1.f}, glm::vec3{0.f, 1.f, 0.f},
        // Bottom face
        glm::vec3{-1.f, -1.f, 1.f}, glm::vec3{0.f, -1.f, 0.f},
        glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{0.f, -1.f, 0.f},
        glm::vec3{1.f, -1.f, -1.f}, glm::vec3{0.f, -1.f, 0.f},
        glm::vec3{1.f, -1.f, 1.f}, glm::vec3{0.f, -1.f, 0.f}};
    const std::array<unsigned int, 36> cube_indices{// Right face
                                                    0, 1, 2, 0, 2, 3,
                                                    // Left face
                                                    4, 5, 6, 4, 6, 7,
                                                    // Front face
                                                    8, 9, 11, 9, 10, 11,
                                                    // Back face
                                                    12, 15, 14, 12, 14, 13,
                                                    // Top face
                                                    16, 17, 19, 17, 18, 19,
                                                    // Bottom face
                                                    20, 21, 22, 20, 22, 23};

    // First bind vao array
    glBindVertexArray(vao);

    // Now bind vbo and submit data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO_INDEX]);
    glBufferData(GL_ARRAY_BUFFER, cube_data.size() * sizeof(glm::vec3),
                 cube_data.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3),
                          reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3),
                          reinterpret_cast<void *>(sizeof(glm::vec3)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO_INDEX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 cube_indices.size() * sizeof(unsigned int),
                 cube_indices.data(), GL_STATIC_DRAW);

    base_program.validate();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
      glClearColor(0.1f, 0.1f, 0.1f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      base_program.use();
      const glm::mat4 M{glm::identity<glm::mat4>()};
      base_program.setMat4("model_matrix", M);
      base_program.setMat3("normal_matrix",
                           glm::transpose(glm::inverse(glm::mat3{M})));
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
      glDrawElements(GL_TRIANGLES, cube_indices.size(), GL_UNSIGNED_INT,
                     nullptr);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteTextures(1, &volume_texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, buffers.data());

    glfwDestroyWindow(window);
    glfwTerminate();

  } catch (const std::exception &ex) {
    spdlog::error(ex.what());
  }

  return 0;
}
