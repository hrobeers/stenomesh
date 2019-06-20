
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
#include <sstream>

#include "stlio.hpp"
#include "plyio.hpp"
#include "mesh.hpp"

using namespace stenomesh;

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

std::istream& binary_read(std::istream &is, std::ostream &os, size_t cnt) {
  for (size_t i=0; i<cnt && !is.eof(); i++)
    os.put(is.get());
  return is;
}

std::istream& binary_read_until(std::istream &is, std::ostream &os, std::string end_line) {
  // TODO hash based?
  std::string line;
  while (std::getline(is, line)) {
    os.write(line.c_str(), line.size());
    os << std::endl;
    if (line.find(end_line)!=std::string::npos)
      break;
  }
  return is;
}

std::string peek_bytes(std::istream &is, size_t cnt) {
  auto pos = is.tellg();
  std::string str(cnt, 0);
  is.read(&str[0], cnt);
  is.seekg(pos);
  return str;
}

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
    Mesh<3> mesh;

    const size_t magic_byte_size = 5;
    std::stringstream header_stream;
    binary_read(std::cin, header_stream, magic_byte_size);
    switch(chash(header_stream.str().c_str(), ' ')) {
    case chash("ply"):
    case chash("PLY"):
      binary_read_until(std::cin, header_stream, "end_header");
      header_stream.seekg(0);
      // TODO continue reading header
      mesh = parsePLY<Mesh<3>>(std::cin, header_stream);
      break;
    case chash("solid"):
      std::cerr << "ASCII STL not supported" << std::endl;
      exit(EXIT_FAILURE);
      break;
    default: // Assume binary STL
      //read until 80 bytes
      while((size_t)header_stream.tellp()<80 && !std::cin.eof())
        header_stream.put(std::cin.get());
      std::cerr << header_stream.str() << std::endl;
      exit(EXIT_FAILURE);
      break;
    }

    //ply.write(std::cout, false);

    /* Other code omitted */

    exit(EXIT_SUCCESS);
  }
  catch (const std::exception & e) {
    std::cerr << "Critical error: " << e.what() << std::endl;
  }
}
