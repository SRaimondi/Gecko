#pragma once

#include "common/helper_macros.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace Geometry {

namespace Internal {

#if __cplusplus >= 201703L

template <typename... B> struct Conjunction {
  static constexpr bool value{std::conjunction_v<B...>};
};

#define NODISCARD [[nodiscard]]

#else

template <typename B1, typename... Bs> struct Conjunction {
  static constexpr bool value{B1::value && Conjunction<Bs...>::value};
};

template <typename B> struct Conjunction<B> {
  static constexpr bool value{B::value};
};

#define NODISCARD

#endif

template <typename Type, typename Function, std::size_t... I>
NODISCARD CUDA_HOST_DEVICE constexpr Type
createFromFuncImplementation(Function &&func, std::index_sequence<I...>) {
  return Type{func(I)...};
}

template <typename Type, const std::size_t N, typename Function,
          typename Indices = std::make_index_sequence<N>>
NODISCARD CUDA_HOST_DEVICE constexpr Type createFromFunc(Function &&func) {
  return createFromFuncImplementation<Type>(std::forward<Function>(func),
                                            Indices{});
}

// Binary reduction helper
template <const std::size_t N, const std::size_t I = N> struct Reduction {
  template <typename ElementFunction, typename ReductionFunction>
  NODISCARD CUDA_HOST_DEVICE static constexpr auto
  runReduction(ElementFunction func, ReductionFunction reduction_func) {
    return reduction_func(
        func(N - I), Reduction<N, I - 1>::runReduction(func, reduction_func));
  }
};

template <const std::size_t N> struct Reduction<N, 2> {
  template <typename ElementFunction, typename ReductionFunction>
  NODISCARD CUDA_HOST_DEVICE static constexpr auto
  runReduction(ElementFunction func, ReductionFunction reduction_func) {
    return reduction_func(func(N - 2), func(N - 1));
  }
};

} // namespace Internal

template <typename T, const std::size_t N> class Vector {
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
  CUDA_HOST_DEVICE explicit constexpr Vector(Args &&... args)
      : elements{std::forward<Args>(args)...} {
    static_assert(sizeof...(Args) == N,
                  "Invalid number of arguments to Vector constructor");
    static_assert(
        Internal::Conjunction<std::is_same<T, std::decay_t<Args>>...>::value,
        "Invalid input arguments to Vector constructor");
  }

  // Cast to another type
  template <typename U>
  NODISCARD CUDA_HOST_DEVICE constexpr Vector<U, N> cast() const {
    static_assert(std::is_convertible<T, U>::value);
    return Internal::createFromFunc<Vector<U, N>, N>(
        [this](const size_type i) -> U {
          return static_cast<U>(this->elements[i]);
        });
  }

  NODISCARD CUDA_HOST_DEVICE static constexpr Vector
  constant(const value_type &v) {
    return Internal::createFromFunc<Vector, N>(
        [v](const size_type) -> value_type { return v; });
  }

  NODISCARD CUDA_HOST_DEVICE static constexpr Vector zero() {
    return constant(value_type{0});
  }

  // Element access
  NODISCARD CUDA_HOST_DEVICE constexpr reference x() noexcept {
    return elements[0];
  }
  NODISCARD CUDA_HOST_DEVICE constexpr const_reference x() const noexcept {
    return elements[0];
  }

  NODISCARD CUDA_HOST_DEVICE constexpr reference y() noexcept {
    static_assert(N >= 2);
    return elements[1];
  }
  NODISCARD CUDA_HOST_DEVICE constexpr const_reference y() const noexcept {
    static_assert(N >= 2);
    return elements[1];
  }

  NODISCARD CUDA_HOST_DEVICE constexpr reference z() noexcept {
    static_assert(N >= 3);
    return elements[2];
  }
  NODISCARD CUDA_HOST_DEVICE constexpr const_reference z() const noexcept {
    static_assert(N >= 3);
    return elements[2];
  }

  NODISCARD CUDA_HOST_DEVICE constexpr reference w() noexcept {
    static_assert(N >= 4);
    return elements[3];
  }
  NODISCARD CUDA_HOST_DEVICE constexpr const_reference w() const noexcept {
    static_assert(N >= 4);
    return elements[3];
  }

  NODISCARD CUDA_HOST_DEVICE constexpr reference
  operator[](const size_type i) noexcept {
    return elements[i];
  }
  NODISCARD CUDA_HOST_DEVICE constexpr const_reference
  operator[](const size_type i) const noexcept {
    return elements[i];
  }

  // Self operations
  CUDA_HOST_DEVICE constexpr Vector &operator+=(const Vector &other) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] += other[i];
    }
    return *this;
  }

  CUDA_HOST_DEVICE constexpr Vector &operator-=(const Vector &other) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] -= other[i];
    }
    return *this;
  }

  CUDA_HOST_DEVICE constexpr Vector &operator*=(const value_type &t) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] *= t;
    }
    return *this;
  }

  CUDA_HOST_DEVICE constexpr Vector &operator/=(const value_type &t) {
    for (size_type i{0}; i != N; ++i) {
      elements[i] /= t;
    }
    return *this;
  }

  // Element wise operations
  NODISCARD CUDA_HOST_DEVICE constexpr Vector
  ewiseProduct(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          return this->operator[](i) * other[i];
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr Vector
  ewiseQuotient(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          return this->operator[](i) / other[i];
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr Vector
  ewiseMin(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          using std::min;
          return min(this->operator[](i), other[i]);
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr Vector
  ewiseMax(const Vector &other) const {
    return Internal::createFromFunc<Vector<T, N>, N>(
        [this, &other](const size_type i) {
          using std::max;
          return max(this->operator[](i), other[i]);
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr Vector ewiseAbs() const {
    return Internal::createFromFunc<Vector<T, N>, N>([this](const size_type i) {
      using std::abs;
      return abs(this->operator[](i));
    });
  }

  NODISCARD CUDA_HOST_DEVICE Vector ewiseExp() const {
    return Internal::createFromFunc<Vector<T, N>, N>([this](const size_type i) {
      using std::exp;
      return exp(this->operator[](i));
    });
  }

  NODISCARD CUDA_HOST_DEVICE Vector cross(const Vector &other) const {
    static_assert(N == 3, "Using cross product on vector with invalid size");
    return Vector{this->y() * other.z() - this->z() * other.y(),
                  this->z() * other.x() - this->x() * other.z(),
                  this->x() * other.y() - this->y() * other.x()};
  }

  // Horizontal operations
  NODISCARD CUDA_HOST_DEVICE constexpr value_type minElement() const {
    return Internal::Reduction<0, N>::runReduction(
        [this](const size_type i) -> value_type { return this->operator[](i); },
        [](const value_type &l, const value_type &r) -> value_type {
          using std::min;
          return min(l, r);
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr value_type maxElement() const {
    return Internal::Reduction<0, N>::runReduction(
        [this](const size_type i) -> value_type { return this->operator[](i); },
        [](const value_type &l, const value_type &r) -> value_type {
          using std::max;
          return max(l, r);
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr value_type
  dot(const Vector &other) const {
    return Internal::Reduction<0, N>::runReduction(
        [this, &other](const size_type i) -> value_type {
          return this->operator[](i) * other[i];
        },
        [](const value_type &l, const value_type &r) -> value_type {
          return l + r;
        });
  }

  NODISCARD CUDA_HOST_DEVICE constexpr value_type squaredNorm() const {
    return Internal::Reduction<0, N>::runReduction(
        [this](const size_type i) -> value_type {
          return this->operator[](i) * this->operator[](i);
        },
        [](const value_type &l, const value_type &r) -> value_type {
          return l + r;
        });
  }

  NODISCARD CUDA_HOST_DEVICE value_type norm() const {
    using std::sqrt;
    return sqrt(squaredNorm());
  }

  NODISCARD CUDA_HOST_DEVICE Vector normalized() const {
    return Internal::createFromFunc<Vector, N>(
        [length{norm()}, this](const size_type i) -> value_type {
          return this->operator[](i) / length;
        });
  }

  NODISCARD CUDA_HOST_DEVICE Vector fastNormalized() const {
    return Internal::createFromFunc<Vector, N>(
        [inv_length{value_type{1} / norm()},
         this](const size_type i) -> value_type {
          return this->operator[](i) * inv_length;
        });
  }

private:
  T elements[N];
};

// Mathematical operations
template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator+(const Vector<T, N> &lhs, const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] + rhs[i];
      });
}

template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator-(const Vector<T, N> &lhs, const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] - rhs[i];
      });
}

// Negate
template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator-(const Vector<T, N> &v) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&v](const typename Vector<T, N>::size_type i) { return -v[i]; });
}

template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator*(const T &lhs, const Vector<T, N> &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [lhs, &rhs](const typename Vector<T, N>::size_type i) {
        return lhs * rhs[i];
      });
}

template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator*(const Vector<T, N> &lhs, const T &rhs) {
  return Internal::createFromFunc<Vector<T, N>, N>(
      [&lhs, rhs](const typename Vector<T, N>::size_type i) {
        return lhs[i] * rhs;
      });
}

template <typename T, const std::size_t N>
NODISCARD CUDA_HOST_DEVICE constexpr Vector<T, N>
operator/(const Vector<T, N> &lhs, const T &rhs) {
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