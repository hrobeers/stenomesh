
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

#ifndef PLYIO_HPP
#define PLYIO_HPP

#include "tinyply.h"
#include <cstring>
using namespace tinyply; // TODO no using in header

// TODO move to generic header?
constexpr
unsigned int chash(const char* str, int h = 0)
{
  return !str[h] ? 5381 : (chash(str, h+1)*33) ^ str[h];
}

template<typename Tmesh>
Tmesh parsePLY(std::istream &is) {
  Tmesh mesh;
  PlyFile ply;

  ply.parse_header(is);
  mesh.comments = ply.get_comments();

  // Tinyply treats parsed data as untyped byte buffers.
  std::shared_ptr<PlyData> vertices, faces;


  for (auto e : ply.get_elements())
    switch (chash(e.name.c_str())) {
    case chash("vertex"):
      // Extract vertices
      vertices = ply.request_properties_from_element("vertex", { "x", "y", "z" });
      break;
    case chash("face"):
      // Extract faces, supporting both "vertex_index" and "vertex_indices"
      for (auto p : e.properties)
        switch (chash(p.name.c_str())) {
        case chash("vertex_index"):
        case chash("vertex_indices"):
          faces = ply.request_properties_from_element(e.name, { p.name }, 3);
        break;
        }
      break;
    }

  if (!vertices) throw std::runtime_error("Failed parsing vertices from input");
  if (!faces) throw std::runtime_error("Failed parsing faces from input");

  ply.read(is);

  const size_t vertices_size = vertices->buffer.size_bytes();
  const size_t vertex_size = vertices_size/vertices->count;
  switch (vertices->t) {
  case tinyply::Type::FLOAT64:
    for (size_t i=0; i<vertices_size; i+=vertex_size) {
      const auto* v = reinterpret_cast<Mesh<3,double>::vertex_t*>(vertices->buffer.get()+i);
      mesh.vertices.push_back({
                               static_cast<float>((*v)[0]),
                               static_cast<float>((*v)[1]),
                               static_cast<float>((*v)[2])
        });
    }
    break;
  case tinyply::Type::FLOAT32:
    mesh.vertices.resize(vertices->count);
    std::memcpy(mesh.vertices.data(), vertices->buffer.get(), vertices_size);
    break;
  }

  const size_t faces_size = faces->buffer.size_bytes();
  const size_t face_size = faces_size/faces->count;
  switch (faces->t) {
  case tinyply::Type::INT32:
    for (size_t i=0; i<faces_size; i+=face_size) {
      const auto* f = reinterpret_cast<Mesh<3>::face_t<int32_t>*>(faces->buffer.get()+i);
      mesh.faces.push_back({
                            static_cast<uint32_t>((*f)[0]),
                            static_cast<uint32_t>((*f)[1]),
                            static_cast<uint32_t>((*f)[2])
        });
    }
    break;
  case tinyply::Type::UINT32:
    mesh.faces.resize(faces->count);
    std::memcpy(mesh.faces.data(), faces->buffer.get(), faces_size);
    break;
  }

  return mesh;
}

#endif // PLYIO_HPP
