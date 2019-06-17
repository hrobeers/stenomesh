
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

#ifndef STLIO_HPP
#define STLIO_HPP

#include <ieee754.h>

// TODO: face streaming?
// stl_stream >> face;
void parseSTL(std::istream &is) {
  // 80 byte header
  std::array<char, 80> header;
  is.read(header.data(), 80);
  uint32_t n_faces;
  is.read(reinterpret_cast<char*>(&n_faces), sizeof(n_faces)); // TODO big endian support

  std::cerr << "header: " << header.data() << std::endl;
  std::cerr << "face cnt: " << n_faces << std::endl;

  for (uint32_t i = 0; i<n_faces && is; i++) {
    std::array<ieee754_float, 3> normal;
    std::array<ieee754_float, 3> v1;
    std::array<ieee754_float, 3> v2;
    std::array<ieee754_float, 3> v3;
    uint16_t attr_size;
    is.read(reinterpret_cast<char*>(&normal), sizeof(normal));
    is.read(reinterpret_cast<char*>(&v1), sizeof(v1));
    is.read(reinterpret_cast<char*>(&v2), sizeof(v2));
    is.read(reinterpret_cast<char*>(&v3), sizeof(v3));

    // the attribute byte count does not signal any byte count.
    // it is used to encode color information (materialise) in just 2 bytes (5bit per color, 32768 colors)
    is.read(reinterpret_cast<char*>(&attr_size), sizeof(attr_size)); // TODO big endian support

    std::cerr << "n:  " << normal[0].f << " " << normal[1].f << " " << normal[2].f << " " << std::endl;
    std::cerr << "v1: " << v1[0].f << " " << v1[1].f << " " << v1[2].f << " " << std::endl;
    std::cerr << "v2: " << v2[0].f << " " << v2[1].f << " " << v2[2].f << " " << std::endl;
    std::cerr << "v3: " << v3[0].f << " " << v3[1].f << " " << v3[2].f << " " << std::endl;
    std::cerr << "a:  " << attr_size << std::endl;
  }
}

#endif // STLIO_HPP