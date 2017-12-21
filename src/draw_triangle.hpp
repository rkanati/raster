
#pragma once

#include <algorithm>
#include <iostream>

#include "image.hpp"
#include "vector.hpp"
#include "line2.hpp"
#include "pixel.hpp"

i32 wf (P2i32 a, P2i32 b, P2i32 p) {
  return (b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x);
}

bool top_left (P2i32 a, P2i32 b) {
  auto const d = b - a;
  return (d.y == 0 && d.x < 0) || d.y > 0;
}

// general triangle rasterizer
template<typename T, typename Shader>
void draw_triangle (
  Image<T>& out,
  P2i32 const a, P2i32 const b, P2i32 const c,
  Shader const& shader)
{
  // twice signed area
  i32 const tsa = wf (a, b, c);
  if (tsa <= 0)
    return;

  // normalizing factor
  float const k = 1.f / tsa;

  // setup
  i32 const
    cx = out.width  () / 2,
    cy = out.height () / 2,
    // bbox, clipped to canvas
    xl = std::max (std::min ({ a.x, b.x, c.x }), -cx  ),
    yl = std::max (std::min ({ a.y, b.y, c.y }), -cy+1),
    xh = std::min (std::max ({ a.x, b.x, c.x }),  cx-1),
    yh = std::min (std::max ({ a.y, b.y, c.y }),  cy  ),
    // edge function deltas
    dwadx = b.y-c.y, dwady = c.x-b.x,
    dwbdx = c.y-a.y, dwbdy = a.x-c.x,
    dwcdx = a.y-b.y, dwcdy = b.x-a.x,
    // edge function biases
    ea = top_left (b, c)? 0 : -1,
    eb = top_left (c, a)? 0 : -1,
    ec = top_left (a, b)? 0 : -1;

  P2i32 p {xl, yl};

  // edge values at left side
  i32
    wa0 = wf (b, c, p) + ea,
    wb0 = wf (c, a, p) + eb,
    wc0 = wf (a, b, p) + ec;

  // sample each pixel in the bbox
  for (; p.y <= yh; p.y++) {
    i32 wa = wa0, wb = wb0, wc = wc0;

    for (p.x = xl; p.x <= xh; p.x++) {
      // shade pixel if we're inside the triangle
      // or on the right kind of edge,
      // passing normalized barycentrics
      if ((wa | wb | wc) >= 0)
        out.at (cx+p.x, cy-p.y) = shader (wa*k, wb*k, wc*k);

      // step edge functions in x
      wa += dwadx; wb += dwbdx; wc += dwcdx;
    }

    // step edge functions in y
    wa0 += dwady; wb0 += dwbdy; wc0 += dwcdy;
  }
}

// draw triangle with no fancy shading
template<typename Pixel, typename Colour>
void draw_triangle_bilevel (Image<Pixel>& out, P2i32 a, P2i32 b, P2i32 c, Colour col) {
  // shader just sets pixels
  auto shader = [col] (auto, auto, auto) { return convert_pixel (col); };
  draw_triangle (out, a, b, c, shader);
}

// draw triangle with interpolated colours
template<typename Pixel, typename Colour>
void draw_triangle_shaded (
  Image<Pixel>& out,
  P2i32 a, Colour a_colour,
  P2i32 b, Colour b_colour,
  P2i32 c, Colour c_colour)
{
  // use barycentric coords to weight vertex colours
  auto shader = [a_colour, b_colour, c_colour] (float a, float b, float c) {
    return convert_pixel (a*a_colour + b*b_colour + c*c_colour);
  };
  draw_triangle (out, a, b, c, shader);
}

