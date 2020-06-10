#include "geometry/vector.hpp"
#include "spdlog/spdlog.h"

int main() {
  spdlog::info("Hello from Gecko project!");

  using namespace Geometry;
  const Vector3f a{Vector3f::constant(2.f)};
  const Vector3f b{Vector3f::constant(3.f)};
  const Vector3f c{a + b};

  return 0;
}
