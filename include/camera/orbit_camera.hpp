#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

#include <algorithm>
#include <utility>

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

  [[nodiscard]] std::pair<glm::vec3, glm::mat4>
  getEyeAndViewMatrix() const noexcept {
    const glm::vec3 v{_radius * std::cos(_phi), _radius * std::cos(_theta),
                      _radius * std::sin(_phi)};
    const glm::vec3 e{_at + v};
    return {e, glm::lookAt(e, _at, glm::vec3{0.f, 1.f, 0.f})};
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

  void moveRight(const float delta) noexcept {
    _at += delta * computeLocalRightVector();
  }

  void moveUp(const float delta) noexcept {
    _at += delta * glm::vec3{0.f, 1.f, 0.f};
  }

  void resetAt() noexcept { _at.x = _at.y = _at.z = 0.f; }

  void changeRadius(const float delta_r,
                    const float min_radius = 0.1f) noexcept {
    _radius = std::max(_radius + delta_r, min_radius);
  }

private:
  constexpr static float PI{3.1415927410125732421875f};
  constexpr static float TWO_PI{2.f * PI};

  [[nodiscard]] glm::vec3 computeLocalRightVector() const noexcept {
    return {std::sin(_phi), 0.f, -std::cos(_phi)};
  }

  [[nodiscard]] glm::vec3 computeLocalUpVector() const noexcept {
    const float cos_theta{std::cos(_theta)};
    return {-cos_theta * std::cos(_phi), std::sin(_theta),
            -cos_theta * std::sin(_phi)};
  }

  glm::vec3 _at;
  float _phi, _theta;
  float _radius;
};

} // namespace Gecko