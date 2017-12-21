
#pragma once

#include "vector.hpp"

template<typename T>
class Line2 final {
  static_assert (std::is_arithmetic<T>::value, "Point component type must be arithmetic");

public:
  Vector<T> n;
  T         d;

  Line2 (Point<T> a, Point<T> b) :
    n (unit (normal_2d (b - a))),
    d (dot (n, a))
  { }
};

template<typename OutStream, typename T>
OutStream& operator << (OutStream& stream, Line2<T> const& line) {
  return stream << line.n << " * " << line.d;
}

template<typename T>
T distance (Point<T> p, Line2<T> line) {
  return dot (p, line.n) - line.d;
}

using Line2i = Line2<int>;
using Line2f = Line2<float>;
using Line2d = Line2<double>;
using Line2i32 = Line2<int32_t>;

