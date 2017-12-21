
#pragma once

#include <limits>
#include <type_traits>

template<typename T>
struct ColourMax {
  static constexpr T value = std::numeric_limits<T>::max ();
};

template<>
struct ColourMax<float> {
  static constexpr float value = 1.0f;
};

template<typename T>
static constexpr T colour_max_v = ColourMax<T>::value;

template<typename T>
class Pixel final {
  static_assert (
    std::is_floating_point<T>::value || std::is_unsigned<T>::value,
    "Pixel components must be floating-point or unsigned integers"
  );

public:
  union {
    struct { T r, g, b, a; };
    T channels[4];
  };

  Pixel (T r, T g, T b, T a = colour_max_v<T>) :
    r (r), g (g), b (b), a (a)
  { }

  Pixel () :
    r (0), g (0), b (0), a (colour_max_v<T>)
  { }
};

// scale pixel
template<typename T>
Pixel<T> operator * (T a, Pixel<T> v) {
  return Pixel<T> (a*v.r, a*v.g, a*v.b, a*v.a);
}

template<typename T>
Pixel<T> operator * (Pixel<T> v, T a) {
  return a * v;
}

// add colours
template<typename T>
Pixel<T> operator + (Pixel<T> a, Pixel<T> b) {
  return Pixel<T> (a.r+b.r, a.g+b.g, a.b+b.b, a.a+b.a);
}

template<typename T>
Pixel<T>& operator += (Pixel<T>& a, Pixel<T> b) {
  a = a + b;
  return a;
}

// common types
using Pixelf = Pixel<float>;
using Pixelu8 = Pixel<uint8_t>;

// require format conversions to be explicit
// HACKHACKHACK: just work for the cases we need
static inline Pixelu8 convert_pixel (Pixelf p) {
  return Pixelu8 (
    uint8_t (p.r*255),
    uint8_t (p.g*255),
    uint8_t (p.b*255),
    uint8_t (p.a*255)
  );
}

static inline uint8_t convert_pixel (uint8_t p) {
  return p;
}

template<typename T>
static inline T convert_pixel (T p) {
  return p;
}

