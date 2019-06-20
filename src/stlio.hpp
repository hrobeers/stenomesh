
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

#include <iostream>
#include <array>

namespace stenomesh {
  // TODO: face streaming?
  // stl_stream >> face;
  template<typename Tmesh>
  Tmesh parseSTL(std::istream &is, std::istream &header_stream) {
    Tmesh mesh;

    // 80 byte header
    std::array<char, 80> header;
    header_stream.read(header.data(), 80);

    uint32_t n_faces;
    is.read(reinterpret_cast<char*>(&n_faces), sizeof(n_faces)); // TODO big endian support

    std::cerr << "header: " << header.data() << std::endl;
    std::cerr << "face cnt: " << n_faces << std::endl;

    for (uint32_t i = 0; i<n_faces && is; i++) {
      std::array<float, 3> normal;
      std::array<float, 3> v1;
      std::array<float, 3> v2;
      std::array<float, 3> v3;
      uint16_t attr_size;
      is.read(reinterpret_cast<char*>(&normal), sizeof(normal));
      is.read(reinterpret_cast<char*>(&v1), sizeof(v1));
      is.read(reinterpret_cast<char*>(&v2), sizeof(v2));
      is.read(reinterpret_cast<char*>(&v3), sizeof(v3));

      size_t idx = mesh.faces.size();
      mesh.faces.push_back({idx, idx+1, idx+2});
      mesh.vertices.push_back(v1);
      mesh.vertices.push_back(v2);
      mesh.vertices.push_back(v3);

      // the attribute byte count does not signal any byte count.
      // it is used to encode color information (materialise) in just 2 bytes (5bit per color, 32768 colors)
      is.read(reinterpret_cast<char*>(&attr_size), sizeof(attr_size)); // TODO big endian support

      std::cerr << "n:  " << normal[0] << " " << normal[1] << " " << normal[2] << " " << std::endl;
      std::cerr << "v1: " << v1[0] << " " << v1[1] << " " << v1[2] << " " << std::endl;
      std::cerr << "v2: " << v2[0] << " " << v2[1] << " " << v2[2] << " " << std::endl;
      std::cerr << "v3: " << v3[0] << " " << v3[1] << " " << v3[2] << " " << std::endl;
      std::cerr << "a:  " << attr_size << std::endl;
    }

    return mesh;
  }

  template<typename Tmesh>
  Tmesh parseSTL(std::istream &is) {
    return parseSTL<Tmesh>(is, is);
  }
}

#endif // STLIO_HPP
