
#pragma once

#include <type_traits>
#include <limits>
#include <cstdint>
#include <cmath>

using i32 = int32_t;

template<typename T>
class Vector final {
  static_assert (std::is_arithmetic<T>::value, "Vector component type must be arithmetic");

public:
  union {
    struct { T x, y, z, w; };
    T components[4];
  };

  Vector (T x, T y, T z = 0) :
    x (x), y (y), z (z), w (1)
  { }

  Vector () :
    x (0), y (0), z (0), w (1)
  { }

  template<typename U>
  Vector (Vector<U> other) :
    x (other.x),
    y (other.y),
    z (other.z),
    w (other.w)
  { }
};

template<typename OutStream, typename T>
OutStream& operator << (OutStream& stream, Vector<T> v) {
  return stream << "(" << v.x << " " << v.y << " " << v.z << " " << v.w << ")";
}

template<typename T>
Vector<T> normal_2d (Vector<T> v) {
  return Vector<T> (-v.y, v.x);
}

template<typename T>
T dot (Vector<T> a, Vector<T> b) {
  return a.x * b.x
       + a.y * b.y
       + a.z * b.z;
}

template<typename T>
Vector<T> operator * (T a, Vector<T> v) {
  return Vector<T> (a*v.x, a*v.y, a*v.z);
}

template<typename T>
Vector<T> operator * (Vector<T> v, T a) {
  return a * v;
}

template<typename T>
Vector<T> operator + (Vector<T> a, Vector<T> b) {
  return Vector<T> (a.x+b.x, a.y+b.y, a.z+b.z);
}

template<typename T>
T length_squared (Vector<T> v) {
  return dot (v, v);
}

template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
T length (Vector<T> v) {
  return std::sqrt (length_squared (v));
}

template<typename T>
Vector<T> unit (Vector<T> v) {
  return v * (T (1) / length (v));
}

template<typename Scalar>
class Point final {
  static_assert (std::is_arithmetic<Scalar>::value, "Point component type must be arithmetic");

public:
  union {
    struct { Scalar x, y, z, w; };
    Scalar components[4];
  };

  Point (Scalar x, Scalar y, Scalar z = 0) :
    x (x), y (y), z (z), w (0)
  { }

  Point () :
    x (0), y (0), z (0), w (0)
  { }

  template<typename U>
  Point (Point<U> other) :
    x (other.x),
    y (other.y),
    z (other.z),
    w (other.w)
  { }
};

template<typename OutStream, typename T>
OutStream& operator << (OutStream& stream, Point<T> p) {
  return stream << "[" << p.x << " " << p.y << " " << p.z << " " << p.w << "]";
}

template<typename T>
Vector<T> operator - (Point<T> p, Point<T> q)  {
  return Vector<T> (p.x-q.x, p.y-q.y, p.z-q.z);
}

template<typename T>
T dot (Point<T> p, Vector<T> v) {
  return p.x * v.x
       + p.y * v.y
       + p.z * v.z;
}

template<typename T>
T dot (Vector<T> v, Point<T> p) {
  return dot (p, v);
}

template<typename T>
T distance_squared (Point<T> a, Point<T> b) {
  return length_squared (b - a);
}

template<typename T>
T distance (Point<T> a, Point<T> b) {
  return length (b - a);
}

using Pointi = Point<int>;
using Pointf = Point<float>;
using Pointd = Point<double>;
using Pointi32 = Point<int32_t>;
using P4i32 = Pointi32;

using Vectori = Vector<int>;
using Vectorf = Vector<float>;
using Vectord = Vector<double>;
using Vectori32 = Vector<int32_t>;
using V4i32 = Vectori32;

template<typename T>
class Vector2 final {
  static_assert (std::is_arithmetic<T>::value, "Vector2 component type must be arithmetic");

public:
  union {
    struct { T x, y; };
    T components[2];
  };

  Vector2 (T x, T y) :
    x (x), y (y)
  { }

  Vector2 () :
    x (0), y (0)
  { }

  template<typename U>
  Vector2 (Vector2<U> other) :
    x (other.x), y (other.y)
  { }
};

template<typename Scalar>
class Point2 final {
  static_assert (std::is_arithmetic<Scalar>::value, "Point2 component type must be arithmetic");

public:
  union {
    struct { Scalar x, y; };
    Scalar components[2];
  };

  Point2 (Scalar x, Scalar y) :
    x (x), y (y)
  { }

  Point2 () :
    x (0), y (0)
  { }

  template<typename U>
  Point2 (Point2<U> other) :
    x (other.x), y (other.y)
  { }
};

template <typename T>
Vector2<T> operator + (Vector2<T> a, Vector2<T> b)
  { return { a.x+b.x, a.y+b.y }; }
template <typename T>
Vector2<T> operator - (Vector2<T> a, Vector2<T> b)
  { return { a.x-b.x, a.y-b.y }; }
template <typename T>
Vector2<T> operator * (Vector2<T> a, Vector2<T> b)
  { return { a.x*b.x, a.y*b.y }; }
template <typename T>
Vector2<T> operator / (Vector2<T> a, Vector2<T> b)
  { return { a.x/b.x, a.y/b.y }; }

template <typename T>
Vector2<T> operator * (Vector2<T> a, T k)
  { return { a.x*k, a.y*k }; }
template <typename T>
Vector2<T> operator / (Vector2<T> a, T k)
  { return { a.x/k, a.y/k }; }

template <typename T>
Vector2<T> operator * (T k, Vector2<T> a)
  { return a * k; }

template <typename T>
Point2<T> operator + (Point2<T> p, Vector2<T> d)
  { return { p.x+d.x, p.y+d.y }; }
template <typename T>
Point2<T> operator - (Point2<T> p, Vector2<T> d)
  { return { p.x-d.x, p.y-d.y }; }
template <typename T>
Vector2<T> operator - (Point2<T> a, Point2<T> b)
  { return { a.x-b.x, a.y-b.y }; }

// fuck it
Point2<float> operator * (Point2<i32> p, float k)
  { return { p.x*k, p.y*k }; }

template<typename T>
Point2<T> operator + (Point2<T> a, Point2<T> b)
  { return { a.x+b.x, a.y+b.y }; }

using P2i32 = Point2 <i32>;
using V2i32 = Vector2<i32>;

