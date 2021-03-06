
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
#include <algorithm>

#include "stlio.hpp"
#include "plyio.hpp"
#include "mesh.hpp"
#include "stringtrim.hpp"
#include "meshproc.hpp"

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
    bool attr = false;
    std::string header;
    std::string steno_msg;
    bool ignore_length = false;
    std::array<float, 3> scale = {1,1,1};
    std::array<float, 3> valid = {0,0,0};
    float collapse_len = NAN;
    float collapse_perc = NAN;

    while ((opt = getopt(argc, argv, "axh:m:f:is:c:p:v:")) != -1) {
      switch (opt) {
      case 'a':
        attr = true;
        break;
      case 'x':
        extract = true;
        break;
      case 'h':
        header = optarg;
        break;
      case 'm':
        steno_msg = optarg;
        break;
      case 'f':
        {
          std::ifstream t(optarg, std::ifstream::in | std::ifstream::binary);

          t.seekg(0, std::ios::end);
          steno_msg.reserve(t.tellg());
          t.seekg(0, std::ios::beg);

          steno_msg.assign((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
        }
        break;
      case 'i':
        ignore_length = true;
        break;
      case 's':
        {
          std::string sarg(optarg);
          char delim = ',';
          float s;

          size_t dim = 0;
          size_t pos = 0;
          while((pos = sarg.find(delim)) != std::string::npos) {
            s = (float)atof(sarg.substr(0,pos).c_str());
            scale[dim++]*=s;
            sarg.erase(0,pos+1);
          }

          s = (float)atof(sarg.c_str());
          while (dim<3)
            scale[dim++]*=s;

          break;
        }
      case 'c':
        collapse_len = (float)atof(optarg);
        break;
      case 'p':
        collapse_perc = (float)atof(optarg);
        break;
      case 'v':
        {
          // TODO deduplicate code with 's'
          std::string sarg(optarg);
          char delim = ',';
          float v;

          size_t dim = 0;
          size_t pos = 0;
          while((pos = sarg.find(delim)) != std::string::npos) {
            v = (float)atof(sarg.substr(0,pos).c_str());
            valid[dim++]=v;
            sarg.erase(0,pos+1);
          }

          v = (float)atof(sarg.c_str());
          while (dim<3)
            valid[dim++]=v;

          break;
        }
      default: /* '?' */
        fprintf(stderr, "usage: %s [-x] [-a] [-h <header_string>] [-m <steno_msg>] [-f <steno_msg_file>] [-s <scale_factor>] [-c <collapse_length>] [-p <collapse_perc_smallest_bbox_edge>] [-v <validation_size>] < meshfile\n",
                argv[0]);
        exit(EXIT_FAILURE);
      }
    }

    if (!attr && (extract || steno_msg.size())) {
      std::cerr << "Only STL attribute encoding is currently supported, use the -a flag as it ensures backwards compatibility." << std::endl;
      exit(EXIT_FAILURE);
    }

    /*
    printf("extract=%d; optind=%d\n",
           extract, optind);

      if (optind >= argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
      }

      std::string meshfile = argv[optind];
      printf("name argument = %s\n", meshfile.c_str());

      std::ifstream fs(meshfile, std::fstream::binary);
    */
    Mesh<3> mesh;
    std::string comment;

    const size_t magic_byte_size = 5;
    std::stringstream header_stream;
    binary_read(std::cin, header_stream, magic_byte_size);
    switch(chash(header_stream.str().c_str(), ' ')) {
    case chash("ply"):
    case chash("PLY"):
      binary_read_until(std::cin, header_stream, "end_header");
      header_stream.seekg(0);
      mesh = parsePLY<Mesh<3>>(std::cin, header_stream);
      break;
    case chash("solid"):
      std::getline(std::cin, comment);
      mesh = parseSTL_ascii<Mesh<3>>(std::cin);
      ltrim(comment);
      mesh.comment = comment;
      break;
    default: // Assume binary STL
      //read until 80 bytes
      while((size_t)header_stream.tellp()<80 && !std::cin.eof())
        header_stream.put(std::cin.get());
      mesh = parseSTL<Mesh<3>>(std::cin, header_stream);
      break;
    }

    // Set the options for writing
    if (header.size()>0)
      mesh.comment = header;
    if (steno_msg.size()>0)
      mesh.steno_msg = steno_msg;

    // Optionally merge close vertices
    // TODO vertex merge currently does not support dist==0
    if (!std::isnan(collapse_len) && collapse_len>0) {
      vertex_merge(mesh, collapse_len);
    }
    if (!std::isnan(collapse_perc) && collapse_perc>0) {
      auto bbox = bounding_box(mesh);
      float min_edge_len = bbox[1].front()-bbox[0].front();
      for (int i=1; i<3; i++) min_edge_len = std::min(min_edge_len, bbox[1][i]-bbox[0][i]);
      // collapse_perc as % of min bbox dim
      vertex_merge(mesh, collapse_perc/100 * min_edge_len);
    }

    if (std::any_of(valid.cbegin(), valid.cend(), [](float f){ return f!=0; })) {
      auto bbox = bounding_box(mesh); // TODO do not recalc if already calculated
      int i=0;
      if (std::any_of(valid.cbegin(), valid.cend(), [&i, &bbox](float f) {
                                                      return bbox[1][i]-bbox[0][i++] < f;
                                                    })) {
        std::cerr << "Mesh validation failed" << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    if (extract)
      std::cout << mesh.steno_msg;
    else
      writeSTL(mesh, scale, std::cout, ignore_length);

    /* Other code omitted */

    exit(EXIT_SUCCESS);
  }
  catch (const std::exception & e) {
    std::cerr << "Critical error: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}
