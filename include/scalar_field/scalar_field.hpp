#pragma once

#include "glm/glm.hpp"

#include <array>
#include <memory>
#include <stdexcept>

namespace Gecko {

template <typename T> class ScalarField {
public:
  ScalarField(const glm::vec3 &bounds_min, const glm::vec3 &bounds_max,
              int x_size, int y_size, int z_size, const T &default_value);

  ScalarField(const ScalarField &other);
  ScalarField(ScalarField &&) noexcept = default;
  ScalarField &operator=(ScalarField &&) noexcept = default;

  ScalarField &operator=(const ScalarField &) = delete;

  [[nodiscard]] const glm::vec3 &min() const noexcept { return _bounds_min; }
  [[nodiscard]] const glm::vec3 &max() const noexcept { return _bounds_max; }

  [[nodiscard]] const glm::vec3 &getVoxelSize() const noexcept {
    return _voxel_size;
  }

  [[nodiscard]] int xSize() const noexcept { return _num_elements.x; }
  [[nodiscard]] int ySize() const noexcept { return _num_elements.y; }
  [[nodiscard]] int zSize() const noexcept { return _num_elements.z; }

  [[nodiscard]] std::size_t totalElements() const noexcept {
    return static_cast<std::size_t>(xSize()) *
           static_cast<std::size_t>(ySize()) *
           static_cast<std::size_t>(zSize());
  }

  [[nodiscard]] T &at(const int i, const int j, const int k) {
    checkIndex(i, j, k);
    return _elements[static_cast<std::size_t>(computeLinearIndex(i, j, k))];
  }

  [[nodiscard]] const T &at(const int i, const int j, const int k) const {
    checkIndex(i, j, k);
    return _elements[static_cast<std::size_t>(computeLinearIndex(i, j, k))];
  }

  [[nodiscard]] T &operator()(const int i, const int j, const int k) noexcept {
    return _elements[static_cast<std::size_t>(computeLinearIndex(i, j, k))];
  }

  [[nodiscard]] const T &operator()(const int i, const int j,
                                    const int k) const noexcept {
    return _elements[static_cast<std::size_t>(computeLinearIndex(i, j, k))];
  }

  [[nodiscard]] glm::vec3 computeDiagonal() const noexcept {
    return max() - min();
  }

  [[nodiscard]] glm::vec3 computeElementPosition(const int i, const int j,
                                                 const int k) const noexcept {
    return computePosition(i, j, k);
  }

  [[nodiscard]] glm::vec3 computeElementPositionSafe(const int i, const int j,
                                                     const int k) const {
    checkIndex(i, j, k);
    return computePosition(i, j, k);
  }

  [[nodiscard]] T *data() noexcept { return _elements.get(); }
  [[nodiscard]] const T *data() const noexcept { return _elements.get(); }

private:
  void checkIndex(const int i, const int j, const int k) const {
    if (i >= xSize() || i < 0 || j >= ySize() || j < 0 || k >= zSize() ||
        k < 0) {
      throw std::out_of_range{"Invalid element index in ScalarField"};
    }
  }

  [[nodiscard]] int computeLinearIndex(const int i, const int j,
                                       const int k) const noexcept {
    return i + xSize() * (j + k * ySize());
  }

  [[nodiscard]] glm::vec3 computePosition(const int i, const int j,
                                          const int k) const noexcept {
    return min() + glm::vec3{static_cast<float>(i), static_cast<float>(j),
                             static_cast<float>(k)} *
                       getVoxelSize();
  }

  glm::vec3 _bounds_min, _bounds_max;
  glm::ivec3 _num_elements;
  glm::vec3 _voxel_size;
  std::unique_ptr<T[]> _elements;
};

template <typename T>
ScalarField<T>::ScalarField(const glm::vec3 &bounds_min,
                            const glm::vec3 &bounds_max, int x_size, int y_size,
                            int z_size, const T &default_value)
    : _bounds_min{bounds_min}, _bounds_max{bounds_max}, _num_elements{x_size,
                                                                      y_size,
                                                                      z_size},
      _voxel_size{computeDiagonal() /
                  glm::vec3{static_cast<float>(_num_elements.x - 1),
                            static_cast<float>(_num_elements.y - 1),
                            static_cast<float>(_num_elements.z - 1)}},
      _elements{new T[totalElements()]} {
  if (_num_elements.x <= 2 || _num_elements.y <= 2 || _num_elements.z <= 2) {
    throw std::runtime_error{"Invalid Scalar field size"};
  }
  if (glm::any(glm::lessThan(_bounds_max, _bounds_min))) {
    throw std::runtime_error{"Scalar field bounds are invalid"};
  }
  for (std::size_t i{0}; i != totalElements(); ++i) {
    _elements[i] = default_value;
  }
}

template <typename T>
ScalarField<T>::ScalarField(const ScalarField &other)
    : _bounds_min{other._bounds_min}, _bounds_max{other._bounds_max},
      _num_elements{other._num_elements},
      _voxel_size{other._voxel_size}, _elements{new T[totalElements()]} {
  for (std::size_t i{0}; i != totalElements(); ++i) {
    _elements[i] = other._elements[i];
  }
}

} // namespace Gecko