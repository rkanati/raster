
#pragma once

#include <cassert>

#include "vector.hpp"

template<typename Pixel>
class Image {
  int const wide, high;
  Pixel* const data;

  int index (P2i32 p) const {
    int i = p.y * wide + p.x;
    assert (i >= 0 && i < size ());
    return i;
  }

public:
  Image (int w, int h) :
    wide (abs (w)),
    high (abs (h)),
    data (new Pixel[wide * high])
  { }

  ~Image () {
    delete[] data;
  }

  int width () const {
    return wide;
  }

  int height () const {
    return high;
  }

  int size () const {
    return wide * high;
  }

  Pixel& at (P2i32 p) {
    return data[index (p)];
  }

  Pixel& at (int x, int y) {
    return at (P2i32 {x, y});
  }

  Pixel at (P2i32 p) const {
    return data[index (p)];
  }

  Pixel at (int x, int y) const {
    return at (P2i32 {x, y});
  }

  Pixel* begin () {
    return data;
  }

  Pixel* end () {
    return data + size ();
  }

  Pixel const* begin () const {
    return data;
  }

  Pixel const* end () const {
    return data + size ();
  }
};

template<typename Pixel>
static void flip_v (Image<Pixel>& im) {
  int h = im.height (),
      w = im.width (),
      l = h / 2;
  for (int y = 0; y != l; y++) {
    for (int x = 0; x != w; x++)
      std::swap (im.at (x, y), im.at (x, h-y-1));
  }
}

