#if defined(WIN32)
#define NOMINMAX
#endif

// clang-format off
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "glutils/utils.hpp"
#include "glutils/program.hpp"
#include "camera/orbit_camera.hpp"
#include "scalar_field/scalar_field.hpp"

#include "spdlog/spdlog.h"

#include <array>

static void glfwErrorCallback(const int error, const char *description) {
  spdlog::error("GLFW error {}: {}", error, description);
}

static Gecko::OrbitCamera camera{glm::vec3{0.f, 0.f, 10.f},
                                 glm::zero<glm::vec3>()};
static glm::vec2 previous_mouse_position;

static uint8_t down_flags{0u};

static void glfwKeyCallback(GLFWwindow *window, const int key,
                            [[maybe_unused]] const int scancode,
                            const int action, [[maybe_unused]] const int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (key == GLFW_KEY_LEFT_SHIFT) {
    if (action == GLFW_PRESS) {
      down_flags |= 0x4u;
    } else if (action == GLFW_RELEASE) {
      down_flags &= ~0x4u;
    }
  }
  if (key == GLFW_KEY_LEFT_CONTROL) {
    if (action == GLFW_PRESS) {
      down_flags |= 0x2u;
    } else if (action == GLFW_RELEASE) {
      down_flags &= ~0x2u;
    }
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    camera.resetAt();
  }
}

static void createPerformanceOverlay() {
  constexpr static float DISTANCE{10.0f};
  const ImVec2 window_pos{DISTANCE, DISTANCE};
  const ImVec2 window_pos_pivot{0.0f, 0.0f};
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  ImGui::SetNextWindowBgAlpha(0.35f);
  constexpr static ImGuiWindowFlags flags{
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove};

  ImGui::Begin("Stats window", nullptr, flags);
  ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)",
              static_cast<double>(1000.0f / ImGui::GetIO().Framerate),
              static_cast<double>(ImGui::GetIO().Framerate));
  ImGui::End();
}

static void glfwMouseButtonCallback(GLFWwindow *window, const int button,
                                    const int action,
                                    [[maybe_unused]] const int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      down_flags |= 0x1u;
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      previous_mouse_position.x = static_cast<float>(x);
      previous_mouse_position.y = static_cast<float>(y);
    } else if (action == GLFW_RELEASE) {
      down_flags &= ~0x1u;
    }
  }
}

static void glfwMouseCallback([[maybe_unused]] GLFWwindow *window,
                              const double xpos, const double ypos) {
  constexpr static float CAMERA_SENSITIVITY{0.01f};
  const bool ctrl_down{static_cast<bool>(down_flags & 0x2u)};
  const bool shift_down{static_cast<bool>(down_flags & 0x4u)};

  if (ctrl_down || shift_down) {
    if (static_cast<bool>(down_flags & 0x1u)) {
      const float x_offset{static_cast<float>(xpos) -
                           previous_mouse_position.x};
      const float y_offset{static_cast<float>(ypos) -
                           previous_mouse_position.y};
      previous_mouse_position.x = static_cast<float>(xpos);
      previous_mouse_position.y = static_cast<float>(ypos);
      if (ctrl_down) {
        camera.rotateVertical(CAMERA_SENSITIVITY * x_offset);
        camera.rotateHorizontal(-CAMERA_SENSITIVITY * y_offset);
      } else {
        camera.moveRight(-CAMERA_SENSITIVITY * x_offset);
        camera.moveUp(CAMERA_SENSITIVITY * y_offset);
      }
    }
  }
}

static void glfwScrollCallback([[maybe_unused]] GLFWwindow *window,
                               [[maybe_unused]] const double xoffset,
                               const double yoffset) {
  constexpr static float SCROLL_SENSITIVITY{0.1f};
  camera.changeRadius(SCROLL_SENSITIVITY * static_cast<float>(-yoffset));
}

template <typename T>
[[nodiscard]] T gaussian(const T &x, const T &a, const T &b, const T &c) {
  const float t{x - b};
  return a * std::exp(-t * t / (2.f * c * c));
}

int main() {
  try {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
      spdlog::error("Could not initialize GLFW");
      return 1;
    }

#if defined(__APPLE__)
    // GL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    if (GL_ARB_debug_output) {
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    }
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create GLFW window
    constexpr static int INITIAL_WIDTH{800};
    constexpr static int INITIAL_HEIGHT{600};
    GLFWwindow *window{glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Gecko",
                                        nullptr, nullptr)};
    if (window == nullptr) {
      spdlog::error("Could not create GLFW window");
      glfwTerminate();
      return 1;
    }
    glfwMakeContextCurrent(window);
    // Enable vsync
    glfwSwapInterval(1);

    constexpr static int MINIMUM_WIDTH{200};
    constexpr static int MINIMUM_HEIGHT{200};
    glfwSetWindowSizeLimits(window, MINIMUM_WIDTH, MINIMUM_HEIGHT,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io{ImGui::GetIO()};
    static_cast<void>(io);

    // Setup dark style
    ImGui::StyleColorsDark();

    // Setup Platform / Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Print message if debugging is enabled
#if !defined(__APPLE__)
    if (GL_ARB_debug_output) {
      glDebugMessageCallback(Gecko::Utils::GLDebugCallback, nullptr);
#if defined(NDEBUG)
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                            GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
#else
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                            nullptr, GL_TRUE);
#endif
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
                           GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                           "Debugging enabled");
    }
#endif

    // Create program
    const Gecko::GLSLProgram volume_render_program{
        Gecko::GLSLShader::createFromFile("../shaders/volume_render.vert"),
        Gecko::GLSLShader::createFromFile("../shaders/volume_render.frag")};

    // Create example field
    using ScalarField = Gecko::ScalarField<float>;
    float field_max{std::numeric_limits<float>::lowest()};
    float field_min{std::numeric_limits<float>::max()};
    ScalarField field{ScalarField::createFromMinMax(
        glm::vec3{-3.f}, glm::vec3{3.f}, 256, 256, 256, 0.f)};

    {
      constexpr static std::array<glm::vec3, 3> exp_centers{
          glm::vec3{-2.f}, glm::vec3{0.f}, glm::vec3{2.f}};
      constexpr static std::array<float, 3> exp_max{10.f, 3.f, 5.f};
      constexpr static std::array<float, 3> exp_c{0.5f, 1.f, 0.8f};

      for (int k{0}; k != field.zSize(); ++k) {
        const float z_pos{field.min().z +
                          static_cast<float>(k) * field.getVoxelSize().z};
        for (int j{0}; j != field.ySize(); ++j) {
          const float y_pos{field.min().y +
                            static_cast<float>(j) * field.getVoxelSize().y};
          for (int i{0}; i != field.xSize(); ++i) {
            const float x_pos{field.min().x +
                              static_cast<float>(i) * field.getVoxelSize().x};
            float value{0.f};
            const glm::vec3 p{x_pos, y_pos, z_pos};
            for (std::size_t g_i{0}; g_i != exp_centers.size(); ++g_i) {
              value =
                  std::max(value, gaussian(glm::length(exp_centers[g_i] - p),
                                           exp_max[g_i], 0.f, exp_c[g_i]));
            }
            if (value > field_max) {
              field_max = value;
            }
            if (value < field_min) {
              field_min = value;
            }
            field(i, j, k) = value;
          }
        }
      }
    }
    spdlog::info("Field max: {}, min: {}", field_max, field_min);

    // Copy data to OpenGL texture
    GLuint volume_texture;
    glGenTextures(1, &volume_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volume_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, field.xSize(), field.ySize(),
                 field.zSize(), 0, GL_RED, GL_FLOAT,
                 reinterpret_cast<void *>(field.data()));
    glBindTexture(GL_TEXTURE_3D, 0);

    volume_render_program.use();
    volume_render_program.setInt("volume_texture", 0);

    // Create tf texture
    std::array<float, 512> tf_data{};
    const float step{(field_max - field_min) /
                     static_cast<float>(tf_data.size() - 1)};
    float current_pos{field_min};
    for (std::size_t i{0}; i != tf_data.size(); ++i) {
    }

    GLuint tf_texture;
    glGenTextures(1, &tf_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, tf_texture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F,
                 static_cast<GLsizei>(tf_data.size()), 0, GL_RED, GL_FLOAT,
                 tf_data.data());

    // Create geometry data
    GLuint vao;
    glGenVertexArrays(1, &vao);
    static constexpr std::size_t VBO_INDEX{0};
    static constexpr std::size_t EBO_INDEX{1};
    std::array<GLuint, 2> buffers{0};
    glGenBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());

    // First bind vao array
    glBindVertexArray(vao);

    // Now bind vbo and submit data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO_INDEX]);
    glBufferData(GL_ARRAY_BUFFER,
                 ScalarField::cube_data.size() * sizeof(glm::vec3),
                 ScalarField::cube_data.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          reinterpret_cast<void *>(0));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO_INDEX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 ScalarField::cube_indices.size() * sizeof(unsigned int),
                 ScalarField::cube_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Window clear color
    glm::vec4 clear_color{0.1f, 0.1f, 0.1f, 1.f};

    // From the field, compute the model matrix
    const glm::mat4 M{field.computeModelMatrix()};
    const glm::mat4 MI{glm::inverse(M)};
    volume_render_program.setFloat("step_size", 0.01f);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      // Clear buffers
      glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Get view matrix
      const auto [eye, V]{camera.getEyeAndViewMatrix()};
      volume_render_program.setVec3("eye_model_space",
                                    glm::vec3{MI * glm::vec4{eye, 1.f}});
      // Update perspective matrix
      int framebuffer_width, framebuffer_height;
      glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
      glViewport(0, 0, framebuffer_width, framebuffer_height);
      const glm::mat4 P{glm::perspectiveFov(
          glm::radians(60.f), static_cast<float>(framebuffer_width),
          static_cast<float>(framebuffer_height), 0.1f, 100.f)};
      volume_render_program.setMat4("MVP", P * V * M);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_3D, volume_texture);

      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES,
                     static_cast<GLsizei>(ScalarField::cube_indices.size()),
                     GL_UNSIGNED_INT, nullptr);

      glBindTexture(GL_TEXTURE_3D, 0);
      glBindVertexArray(0);

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      createPerformanceOverlay();

      // Render
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    glDeleteTextures(1, &volume_texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

  } catch (const std::exception &ex) {
    spdlog::error(ex.what());
  }

  return 0;
}
