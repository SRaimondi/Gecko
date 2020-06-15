#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace Geometry {
namespace Internal {

template <typename T, const std::size_t N>
[[nodiscard]] constexpr std::size_t computeAlignment() noexcept {
  if (std::is_fundamental<T>::value) {
    const std::size_t total_size{sizeof(T) * N};
    if (total_size % 64 == 0) {
      return 64;
    } else if (total_size % 32 == 0) {
      return 32;
    } else if (total_size % 16 == 0) {
      return 16;
    }
  }
  return std::alignment_of<T>::value;
}

template <typename Type, typename Function, std::size_t... I>
[[nodiscard]] constexpr Type
createFromFuncImplementation(Function &&func, std::index_sequence<I...>) {
  return Type{func(I)...};
}

template <typename Type, const std::size_t N, typename Function,
          typename Indices = std::make_index_sequence<N>>
[[nodiscard]] constexpr Type createFromFunc(Function &&func) {
  return createFromFuncImplementation<Type>(std::forward<Function>(func),
                                            Indices{});
}

// Binary reduction helper
template <const std::size_t N, const std::size_t I = N> struct Reduction {
  template <typename ElementFunction, typename ReductionFunction>
  [[nodiscard]] static constexpr auto
  runReduction(ElementFunction func, ReductionFunction reduction_func) {
    return reduction_func(
        func(N - I), Reduction<N, I - 1>::runReduction(func, reduction_func));
  }
};

template <const std::size_t N> struct Reduction<N, 2> {
  template <typename ElementFunction, typename ReductionFunction>
  [[nodiscard]] static constexpr auto
  runReduction(ElementFunction func, ReductionFunction reduction_func) {
    return reduction_func(func(N - 2), func(N - 1));
  }
};

template <const std::size_t I> struct Reduction<1, I> {
  template <typename ElementFunction, typename ReductionFunction>
  [[nodiscard]] static constexpr auto runReduction(ElementFunction func,
                                                   ReductionFunction) {
    return func(0);
  }
};

} // namespace Internal

template <typename T, const std::size_t N>
class alignas(Internal::computeAlignment<T, N>()) Vector {
public:
  using value_type = T;
  using size_type = unsigned int;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = value_type *;
  using const_iterator = const value_type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Compile time size
  static constexpr size_type SIZE{N};

  // Construction
  template <typename... Args>
  explicit constexpr Vector(Args &&... args)
      : elements{std::forward<Args>(args)...} {
    static_assert(sizeof...(Args) == N,
                  "Invalid number of arguments to Vector constructor");
    static_assert(std::conjunction_v<std::is_same_v<T, std::decay_t<Args>>...>,
                  "Invalid input arguments to Vector constructor");
  }

  // Cast to another type
  template <typename U> [[nodiscard]] constexpr Vector<U, N> cast() const {
    static_assert(std::is_convertible<T, U>::value, "Invalid casting");
    return Internal::createFromFunc<Vector<U, N>, N>(
        [this](const size_type i) -> U {
          return static_cast<U>(this->elements[i]);
        });
  }

  [[nodiscard]] static constexpr Vector constant(const value_type &v) {
    return Internal::createFromFunc<Vector, N>(
        [v](const size_type) -> value_type { return v; });
  }

  [[nodiscard]] static constexpr Vector zero() {
    return constant(value_type{0});
  }

  // Element access
  [[nodiscard]] constexpr reference x() noexcept { return elements[0]; }
  [[nodiscard]] constexpr const_reference x() const noexcept {
    return elements[0];
  }

  [[nodiscard]] constexpr reference y() noexcept {
    static_assert(N >= 2, "Using y() member access on vector of size < 2");
    return elements[1];
  }
  [[nodiscard]] constexpr const_reference y() const noexcept {
    static_assert(N >= 2, "Using y() member access on vector of size < 2");
    return elements[1];
  }

  [[nodiscard]] constexpr reference z() noexcept {
    static_assert(N >= 3, "Using z() member access on vector of size < 3");
    return elements[2];
  }
  [[nodiscard]] constexpr const_reference z() const noexcept {
    static_assert(N >= 3, "Using z() member access on vector of size < 3");
    return elements[2];
  }

  [[nodiscard]] constexpr reference w() noexcept {
    static_assert(N >= 4, "Using w() member access on vector of size < 4");
    return elements[3];
  }
  [[nodiscard]] constexpr const_reference w() const noexcept {
    static_assert(N >= 4, "Using w() member access on vector of size < 4");
    return elements[3];
  }

  [[nodiscard]] constexpr reference operator[](const size_type i) noexcept {
    return elements[i];
  }
  [[nodiscard]] constexpr const_reference
  operator[](const size_type i) const noexcept {
    return elements[i];
  }

  // Self operations
  constexpr Vector &operator+=(const Vector &other) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] += other[i];
    }
    return *this;
  }

  constexpr Vector &operator-=(const Vector &other) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] -= other[i];
    }
    return *this;
  }

  constexpr Vector &operator*=(const value_type &t) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] *= t;
    }
    return *this;
  }

  constexpr Vector &operator/=(const value_type &t) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] /= t;
    }
    return *this;
  }

  // Element wise operations
  [[nodiscard]] constexpr Vector ewiseProduct(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          return this->operator[](i) * other[i];
        });
  }

  [[nodiscard]] constexpr Vector ewiseQuotient(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          return this->operator[](i) / other[i];
        });
  }

  [[nodiscard]] constexpr Vector ewiseMin(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          using std::min;
          return min(this->operator[](i), other[i]);
        });
  }

  [[nodiscard]] constexpr Vector ewiseMax(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          using std::max;
          return max(this->operator[](i), other[i]);
        });
  }

  [[nodiscard]] constexpr Vector ewiseAbs() const {
    return Internal::createFromFunc<Vector<T, N>, N>([this](const size_type i) {
      using std::abs;
      return abs(this->operator[](i));
    });
  }

  [[nodiscard]] Vector ewiseExp() const {
    return Internal::createFromFunc<Vector<T, N>, N>([this](const size_type i) {
      using std::exp;
      return exp(this->operator[](i));
    });
  }

  [[nodiscard]] Vector cross(const Vector &other) const {
    static_assert(N == 3, "Using cross product on vector with invalid size");
    return Vector{this->y() * other.z() - this->z() * other.y(),
                  this->z() * other.x() - this->x() * other.z(),
                  this->x() * other.y() - this->y() * other.x()};
  }

  // Horizontal operations
  [[nodiscard]] constexpr value_type minElement() const {
    return Internal::Reduction<N>::runReduction(
        [this](const size_type i) -> value_type { return this->operator[](i); },
        [](const value_type &l, const value_type &r) -> value_type {
          using std::min;
          return min(l, r);
        });
  }

  [[nodiscard]] constexpr value_type maxElement() const {
    return Internal::Reduction<N>::runReduction(
        [this](const size_type i) -> value_type { return this->operator[](i); },
        [](const value_type &l, const value_type &r) -> value_type {
          using std::max;
          return max(l, r);
        });
  }

  [[nodiscard]] constexpr value_type dot(const Vector &other) const {
    return Internal::Reduction<N>::runReduction(
        [this, &other](const size_type i) -> value_type {
          return this->operator[](i) * other[i];
        },
        [](const value_type &l, const value_type &r) -> value_type {
          return l + r;
        });
  }

  [[nodiscard]] constexpr value_type squaredNorm() const {
    return Internal::Reduction<N>::runReduction(
        [this](const size_type i) -> value_type {
          return this->operator[](i) * this->operator[](i);
        },
        [](const value_type &l, const value_type &r) -> value_type {
          return l + r;
        });
  }

  [[nodiscard]] value_type norm() const {
    using std::sqrt;
    return sqrt(squaredNorm());
  }

  [[nodiscard]] Vector normalized() const {
    return Internal::createFromFunc<Vector, N>(
        [length{norm()}, this](const size_type i) -> value_type {
          return this->operator[](i) / length;
        });
  }

  [[nodiscard]] Vector fastNormalized() const {
    return Internal::createFromFunc<Vector, N>(
        [inv_length{value_type{1} / norm()},
         this](const size_type i) -> value_type {
          return this->operator[](i) * inv_length;
        });
  }

private:
  T elements[N];
};

// Avoid 0-size vector
template <typename T> class Vector<T, 0>;

// Mathematical operations
template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator+(const Vector<T, N> &lhs,
                                               const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] + rhs[i];
      });
}

template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator-(const Vector<T, N> &lhs,
                                               const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] - rhs[i];
      });
}

// Negate
template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator-(const Vector<T, N> &v) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&v](const typename Vector<T, N>::size_type i) { return -v[i]; });
}

template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator*(const T &lhs,
                                               const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs * rhs[i];
      });
}

template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator*(const Vector<T, N> &lhs,
                                               const T &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] * rhs;
      });
}

template <typename T, const std::size_t N>
[[nodiscard]] constexpr Vector<T, N> operator/(const Vector<T, N> &lhs,
                                               const T &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] / rhs;
      });
}

// Common types
template <typename T> using Vector2 = Vector<T, 2>;
template <typename T> using Vector3 = Vector<T, 3>;
template <typename T> using Vector4 = Vector<T, 4>;

#define CREATE_TYPES(SIZE)                                                     \
  using Vector##SIZE##i = Vector##SIZE<int>;                                   \
  using Vector##SIZE##ui = Vector##SIZE<unsigned int>;                         \
  using Vector##SIZE##l = Vector##SIZE<long>;                                  \
  using Vector##SIZE##ll = Vector##SIZE<long long>;                            \
  using Vector##SIZE##ul = Vector##SIZE<unsigned long>;                        \
  using Vector##SIZE##st = Vector##SIZE<std::size_t>;                          \
  using Vector##SIZE##f = Vector##SIZE<float>;                                 \
  using Vector##SIZE##d = Vector##SIZE<double>;

CREATE_TYPES(2)
CREATE_TYPES(3)
CREATE_TYPES(4)

#undef CREATE_TYPES

} // namespace Geometry
