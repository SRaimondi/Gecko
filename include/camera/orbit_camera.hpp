#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

#include <algorithm>

namespace Gecko {

class OrbitCamera {
public:
  OrbitCamera() noexcept;

  OrbitCamera(const glm::vec3 &from, const glm::vec3 &at) noexcept;

  [[nodiscard]] glm::mat4 getViewMatrix() const noexcept {
    const glm::vec3 v{_radius * std::cos(_phi), _radius * std::cos(_theta),
                      _radius * std::sin(_phi)};
    return glm::lookAt(_at + v, _at, glm::vec3{0.f, 1.f, 0.f});
  }

  void rotateVertical(const float delta_phi) noexcept {
    const float new_phi{_phi + delta_phi};
    if (new_phi > TWO_PI) {
      _phi = new_phi - TWO_PI;
    } else if (new_phi < 0.f) {
      _phi = TWO_PI + new_phi;
    } else {
      _phi = new_phi;
    }
  }

  void rotateHorizontal(const float delta_theta) noexcept {
    _theta = std::clamp(_theta + delta_theta, 0.f, PI);
  }

  void setLookAt(const glm::vec3 &at) noexcept { _at = at; }

  void changeRadius(const float delta_r) noexcept { _radius += delta_r; }

private:
  constexpr static float PI{3.1415927410125732421875f};
  constexpr static float TWO_PI{2.f * PI};

  glm::vec3 _at;
  float _phi, _theta;
  float _radius;
};

} // namespace Gecko