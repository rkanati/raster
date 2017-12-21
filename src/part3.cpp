
#include <limits>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <system_error>
#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>

#include "vector.hpp"
#include "image.hpp"
#include "draw_triangle.hpp"

// program options
class Options {
public:
//char const* script_path = nullptr;
  char const* output_path = "out.ppm";
};

// extract program options from command line arguments
Options parse_options (char const** args, int arg_count) {
  Options opts;

  for (int i = 1; i != arg_count; i++) {
    char const* arg = args[i];

    if (!strcmp ("--output", arg) || !strcmp ("-o", arg)) {
      if (++i == arg_count || args[i][0] == '-')
        throw std::runtime_error ("Need output path");
      opts.output_path = args[i];
    }
  /*else {
      if (!strcmp ("--script", arg) || !strcmp ("-s", arg)) {
        if (++i == arg_count || args[i][0] == '-')
          throw std::runtime_error ("Need script path");
      }
      opts.script_path = args[i];
    }*/
  }

/*if (!opts.script_path)
    throw std::runtime_error ("No script specified");*/

  return opts;
}

// in-memory representation of scene described by script
struct Vertex {
  P2i32 position;
  P2i32 uv;
};

struct Triangle {
  Vertex verts[3];
  Triangle (Vertex a, Vertex b, Vertex c) : verts{a,b,c} { }
};

/*struct Script {
  int width, height;
  std::vector<Triangle> triangles;
};*/

// tokens - used for lexical analysis
/*enum class TokenType {
  word,
  number,
  colour,
  line_end
};

struct Token {
  TokenType type;
  char const* spelling;
  char const* end;
  size_t length () const {
    return end - spelling;
  }
};

// paranoid character class checking for lexer
bool is_letter (char c) {
  return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

bool is_digit (char c) {
  return c>='0' && c<='9';
}

bool is_hex_digit (char c) {
  return is_digit (c) || (c>='a' && c<= 'f') || (c>='A' && c<='F');
}

// avoid strchr - it considers the terminating null as part of the string
bool contains (char const* set, char c) {
  while (*set) {
    if (*set++ == c)
      return true;
  }
  return false;
}

// tokenizes source code
std::vector<Token> lex_script (char const* source, size_t length) {
  std::vector<Token> tokens;

  char const* end = source + length;
  char const* ptr = source;

  while (ptr != end) {
    char const* begin = ptr;

    // skip whitespace
    if (contains (" \t", *ptr)) {
      ptr++;
      continue;
    }

    // lex line endings
    if (contains ("\r\n", *ptr)) {
      ptr++;

      // handle crlf
      if (ptr != end && *begin == '\r' && *ptr == '\n')
        ptr++;

      tokens.push_back (Token { TokenType::line_end, begin, ptr });
      continue;
    }

    // skip comments
    if (*ptr == '\'') {
      do { ptr++; } while (ptr != end && !contains ("\r\n", *ptr));
      continue;
    }

    // lex words
    if (is_letter (*ptr)) {
      do { ptr++; } while (ptr != end && is_letter (*ptr));
      tokens.push_back (Token { TokenType::word, begin, ptr });
      continue;
    }

    // lex numbers
    if (is_digit (*ptr) || contains ("+-", *ptr)) {
      do { ptr++; } while (ptr != end && is_digit (*ptr));
      tokens.push_back (Token { TokenType::number, begin, ptr });
      continue;
    }

    // lex colours
    if (*ptr == '#') {
      do { ptr++; } while (ptr != end && is_hex_digit (*ptr));
      if (ptr - begin != 7) // one # plus six digits
        throw std::runtime_error ("Invalid colour");
      tokens.push_back (Token { TokenType::colour, begin, ptr });
      continue;
    }

    throw std::runtime_error ("Invalid characters in script");
  }

  return tokens;
}

// parses an integer, and returns its value
int parse_number (char const* spelling, char const* end) {
  assert (end > spelling);

  std::istringstream stream (std::string (spelling, end));
  int value = 0;
  stream >> value;
  return value;
}

// parse a #rrggbb style hex colour
Pixelf parse_colour (char const* spelling, char const* end) {
  assert (end - spelling == 7);

  std::istringstream stream (std::string (spelling+1, end));
  int raw = 0;
  stream >> std::hex >> raw;

  Pixelf colour;
  for (int i = 0; i != 3; i++) {
    uint8_t bits = (raw >> ((2-i)*8)) & 0xff;
    colour.channels[i] = int (bits) * (1.0f/255);
  }
  return colour;
}

// for reporting parse errors
class ParseError : public std::runtime_error {
public:
  template<typename Arg>
  ParseError (Arg&& arg) : std::runtime_error (std::forward<Arg> (arg)) { }
};

// parses a #colour x y vertex from the given tokens
Token const* parse_vertex (Vertex& vertex, Token const* token, Token const* end) {
  // colour first
  if (token == end || token->type != TokenType::colour)
    throw ParseError ("Expected vertex colour");
  vertex.colour = parse_colour (token->spelling, token->end);
  token++;

  // then coords
  for (int i = 0; i != 2; i++) {
    if (token == end || token->type != TokenType::number)
      throw ParseError ("Expected vertex coordinate");
    vertex.position.components[i] = parse_number (token->spelling, token->end);
    token++;
  }

  return token;
}

// parses a triangle command
Token const* parse_triangle (Script& script, Token const* arguments, Token const* end) {
  Triangle tri;
  Token const* ptr = arguments;

  // three vertices
  for (int i = 0; i != 3; i++)
    ptr = parse_vertex (tri.verts[i], ptr, end);

  script.triangles.push_back (tri);
  return ptr;
}

// parses a canvas command
Token const* parse_canvas (Script& script, Token const* arguments, Token const* end) {
  Token const* width_spec = arguments;
  if (width_spec == end || width_spec->type != TokenType::number)
    throw ParseError ("Expected width specification in canvas command");
  script.width = parse_number (width_spec->spelling, width_spec->end);

  Token const* height_spec = arguments+1;
  if (height_spec == end || height_spec->type != TokenType::number)
    throw ParseError ("Expect height specification in canvas command");
  script.height = parse_number (height_spec->spelling, height_spec->end);

  return arguments+2;
}

// parse tokenized source into a script object
Script parse_script (std::vector<Token> const& tokens) {
  Script script;

  Token const* token = tokens.data ();
  Token const* end   = token + tokens.size ();

  while (token != end) {
    // skip blank lines
    if (token->type == TokenType::line_end) {
      token++;
    }
    // parse commands
    else if (token->type == TokenType::word) {
      if (!strncmp (token->spelling, "canvas", token->length ()))
        token = parse_canvas (script, token+1, end);
      else if (!strncmp (token->spelling, "triangle", token->length ()))
        token = parse_triangle (script, token+1, end);
      else
        throw ParseError ("Unknown command");

      // every command ends with a line break
      if (token != end && token->type != TokenType::line_end)
        throw ParseError ("Expected newline after command");
      token++;
    }
    else {
      throw ParseError ("Expected command");
    }
  }

  return script;
}
*/
// load a file in one go
std::vector<char> load_file (char const* path) {
  std::ifstream file (path);

  file.seekg (0, std::ios_base::end);
  auto size = file.tellg ();
  file.seekg (0, std::ios_base::beg);
  std::vector<char> buffer (size);

  file.read (buffer.data (), size);
  return buffer;
}

// load and process a script
/*Script load_script (char const* path) {
  auto source = load_file (path);
  auto tokens = lex_script (source.data (), source.size ());
  return parse_script (tokens);
}*/

static constexpr uint8_t const tex[8][8] = {
  { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f },
  { 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f },
  { 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f },
  { 0x7f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f },
  { 0x7f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f },
  { 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f },
  { 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f },
  { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f }
};

void draw_triangle (Image<Pixelu8>& canvas, Triangle const& tri) {
  auto shader = [&tri] (float a, float b, float c) {
    Point2<float> uv = tri.verts[0].uv*a + tri.verts[1].uv*b + tri.verts[2].uv*c;
    uint8_t t = tex[7-(int(uv.y+.5f)&7)][int(uv.x+.5f)&7];
    return Pixelu8{t,t,t};
  };

  draw_triangle (
    canvas,
    tri.verts[0].position,
    tri.verts[1].position,
    tri.verts[2].position,
    shader
  );
}

// compute an 8-bit image from a script object
Image<Pixelu8> rasterize (int w, int h, std::vector<Triangle> const& triangles) {
  Image<Pixelu8> canvas (w, h);
  for (Triangle const& triangle : triangles)
    draw_triangle (canvas, triangle);
  return canvas;
}

// save image as a .ppm
void write_image (char const* path, Image<Pixelu8> const& image) {
  std::ofstream out (path, std::ios_base::binary);
  out << "P6\n" << image.width () << " " << image.height () << "\n"
      << "255\n";
  for (Pixelu8 const& pixel : image)
    out.write ((char const*) pixel.channels, 3);
}

int main (int arg_count, char const** args) try {
  auto opts = parse_options (args, arg_count);
//auto script = load_script (opts.script_path);
  std::vector<Triangle> tris {
    Triangle { { {-400,-400}, { 0, 0} }, { { 400,-400}, {64, 0} }, { {-400, 400}, {0, 64} } },
    Triangle { { { 400, 400}, {64,64} }, { {-400, 400}, { 0,64} }, { { 400,-400}, {64, 0} } }
/*  Triangle { { {   0, -400}, {0,0} }, { {  120, -160}, {64,0} }, { { -120, -160}, {64,64} } },
    Triangle { { {-380, -124}, {0,0} }, { { -120, -160}, {64,0} }, { { -190,   60}, {64,64} } },
    Triangle { { { 380, -124}, {0,0} }, { {  190,   60}, {64,0} }, { {  120, -160}, {64,64} } },
    Triangle { { {-236,  324}, {0,0} }, { { -190,   60}, {64,0} }, { {    0,  200}, {64,64} } },
    Triangle { { { 236,  324}, {0,0} }, { {    0,  200}, {64,0} }, { {  190,   60}, {64,64} } },
    Triangle { { {   0,    0}, {0,0} }, { { -120, -160}, {64,0} }, { {  120, -160}, {64,64} } },
    Triangle { { {   0,    0}, {0,0} }, { { -190,   60}, {64,0} }, { { -120, -160}, {64,64} } },
    Triangle { { {   0,    0}, {0,0} }, { {  120, -160}, {64,0} }, { {  190,   60}, {64,64} } },
    Triangle { { {   0,    0}, {0,0} }, { {    0,  200}, {64,0} }, { { -190,   60}, {64,64} } },
    Triangle { { {   0,    0}, {0,0} }, { {  190,   60}, {64,0} }, { {    0,  200}, {64,64} } }*/
  };
  auto image = rasterize (1024, 1024, tris);
  write_image (opts.output_path, image);
}
catch (std::exception const& e) {
  std::cerr << e.what () << "\n";
  return 1;
}

