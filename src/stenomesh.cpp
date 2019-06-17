
// Copyright (C) 2019 hrobeers (https://github.com/hrobeers)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#include <array>
#include <vector>



template<size_t N, typename Tflt = float>
struct Mesh
{
  typedef Tflt float_t;
  typedef std::array<Tflt,3> vertex_t;
  typedef std::vector<vertex_t> vertices_t;

  template <typename T = uint32_t>
  using face_t = std::array<T, N>;

  template <typename T = uint32_t>
  using faces_t = std::vector<face_t<T>>;

  vertices_t vertices;
  faces_t<> faces;
  std::vector<std::string> comments;
};

#include "stlio.hpp"
#include "plyio.hpp"

int main(int argc, char **argv)
{
  try {
    /* avoid end-of-line conversions */
    SET_BINARY_MODE(stdin);
    SET_BINARY_MODE(stdout);

    /* parse commandline options */
    int opt;
    bool extract = false;

    while ((opt = getopt(argc, argv, "x")) != -1) {
      switch (opt) {
      case 'x':
        extract = true;
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-x] meshfile\n",
                argv[0]);
        exit(EXIT_FAILURE);
      }
    }

    printf("extract=%d; optind=%d\n",
           extract, optind);

    /*
      if (optind >= argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
      }

      std::string meshfile = argv[optind];
      printf("name argument = %s\n", meshfile.c_str());

      std::ifstream fs(meshfile, std::fstream::binary);
    */

    //parseSTL(std::cin);
    auto mesh = parsePLY<Mesh<3>>(std::cin);
    //auto mesh = parsePLY(fs);

    //ply.write(std::cout, false);

    /* Other code omitted */

    exit(EXIT_SUCCESS);
  }
  catch (const std::exception & e) {
    std::cerr << "Critical error: " << e.what() << std::endl;
  }
}
