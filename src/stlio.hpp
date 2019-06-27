
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
#include <sstream>
#include <limits>

#include "vertexio.hpp"

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

    union {
      uint32_t num;
      char data[sizeof(uint32_t)];
    } steno_msg_size;
    std::stringstream attr_stream;

    std::array<float, 3> normal;
    std::array<float, 3> v1;
    std::array<float, 3> v2;
    std::array<float, 3> v3;
    std::array<char, 2> attr;

    for (uint32_t i = 0; i<n_faces && is; i++) {
      is.read(reinterpret_cast<char*>(&normal), sizeof(normal));
      is.read(reinterpret_cast<char*>(&v1), sizeof(v1));
      is.read(reinterpret_cast<char*>(&v2), sizeof(v2));
      is.read(reinterpret_cast<char*>(&v3), sizeof(v3));

      size_t idx = mesh.faces.size()*3;
      mesh.faces.push_back({idx, idx+1, idx+2});
      mesh.vertices.push_back(v1);
      mesh.vertices.push_back(v2);
      mesh.vertices.push_back(v3);

      // the attribute byte count does not signal any byte count.
      // it is used to encode color information (materialise) in just 2 bytes (5bit per color, 32768 colors)
      // stenomesh uses it to store steno messages
      is.read(attr.data(), sizeof(attr));
      if (i*2<sizeof(steno_msg_size.num)) {
        steno_msg_size.data[i*2]  = attr[0];
        steno_msg_size.data[i*2+1] = attr[1];
      }
      else if (i*2<steno_msg_size.num+sizeof(steno_msg_size.num)){
        attr_stream.put(attr[0]);
        attr_stream.put(attr[1]);
      }
    }

    mesh.steno_msg = attr_stream.str().substr(0,steno_msg_size.num);

    return mesh;
  }

  template<typename Tmesh>
  Tmesh parseSTL(std::istream &is) {
    return parseSTL<Tmesh>(is, is);
  }

  inline std::istream& read_until(std::istream &is, const char* str) {
    size_t idx = 0;
    while (!is.eof()) {
      char c = is.get();
      if (str[idx]==0)
        return is;
      if (str[idx]==c)
        idx++;
      else
        idx=0;
    }
    return is;
  }

  template<typename Tmesh>
  Tmesh parseSTL_ascii(std::istream &is) {
    Tmesh mesh;

    std::array<float, 3> normal;
    std::array<float, 3> v1;
    std::array<float, 3> v2;
    std::array<float, 3> v3;

    while (!is.eof()) {
      // read_next_vertex(std::istream& str, vertex_t& v)
      read_until(is, "normal");
      vertexio::read_next_vertex(is, normal);
      read_until(is, "vertex");
      vertexio::read_next_vertex(is, v1);
      read_until(is, "vertex");
      vertexio::read_next_vertex(is, v2);
      read_until(is, "vertex");
      vertexio::read_next_vertex(is, v3);

      size_t idx = mesh.faces.size()*3;
      mesh.faces.push_back({idx, idx+1, idx+2});
      mesh.vertices.push_back(v1);
      mesh.vertices.push_back(v2);
      mesh.vertices.push_back(v3);
    }

    return mesh;
  }

  template<typename Tmesh>
  std::ostream& writeSTL(Tmesh mesh, std::ostream &os) {
    std::array<char,80> header;
    header.fill(0);
    mesh.comment.copy(header.data(), 80);
    os.write(header.data(), 80);

    uint32_t face_cnt = mesh.faces.size();
    os.write(reinterpret_cast<char*>(&face_cnt), sizeof(face_cnt));

    if (mesh.steno_msg.size()>std::min(face_cnt*2-(int)sizeof(uint32_t),std::numeric_limits<uint32_t>::max()))
      throw std::runtime_error("Steno message overflows the available storage space");
    std::stringstream steno_stream;
    uint32_t msg_size = mesh.steno_msg.size();
    steno_stream.write(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
    steno_stream.write(mesh.steno_msg.c_str(),msg_size);

    std::array<float,3> dummy_normal = {0,0,0};

    std::array<char,2> attr_byte_cnt;

    for (auto f : mesh.faces) {
      os.write(reinterpret_cast<char*>(dummy_normal.data()), sizeof(dummy_normal));
      os.write(reinterpret_cast<char*>(&(mesh.vertices[f[0]])), sizeof(dummy_normal));
      os.write(reinterpret_cast<char*>(&(mesh.vertices[f[1]])), sizeof(dummy_normal));
      os.write(reinterpret_cast<char*>(&(mesh.vertices[f[2]])), sizeof(dummy_normal));

      attr_byte_cnt.fill(-1); // -1 = white according to meshlab
      if (!steno_stream.eof())
        steno_stream.read(attr_byte_cnt.data(),2);
      os.write(attr_byte_cnt.data(), sizeof(attr_byte_cnt));
    }

    return os;
  }
}

#endif // STLIO_HPP
