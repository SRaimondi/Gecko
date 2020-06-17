#include "camera/orbit_camera.hpp"

#include <cmath>

namespace Gecko {

OrbitCamera::OrbitCamera() noexcept
    : _at{glm::vec3{0.f}}, _phi{glm::radians(90.f)}, _theta{glm::radians(90.f)},
      _radius{1.f} {}

OrbitCamera::OrbitCamera(const glm::vec3 &from, const glm::vec3 &at) noexcept
    : _at{at}, _phi{[at_to_from{from - at}]() -> float {
        return std::atan2(at_to_from.z, at_to_from.x);
      }()},
      _theta{[at_to_from{from - at}]() -> float {
        return std::acos(at_to_from.y / glm::length(at_to_from));
      }()},
      _radius{glm::length(from - at)} {}

} // namespace Gecko