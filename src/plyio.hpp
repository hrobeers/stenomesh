
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


namespace stenomesh {
  // TODO move to generic header?
  constexpr
  unsigned int chash(const char* str, int h = 0)
  {
    return !str[h] ? 5381 : (chash(str, h+1)*33) ^ str[h];
  }

  // http://loungecpp.wikidot.com/tips-and-tricks%3aindices
  //#include <array>
  //#include <type_traits>
  template <std::size_t... Is>
  struct indices {};
  template <std::size_t N, std::size_t... Is>
  struct build_indices: build_indices<N-1, N-1, Is...> {};
  template <std::size_t... Is>
  struct build_indices<0, Is...>: indices<Is...> {};

  template<typename T, typename U, size_t i, size_t... Is>
  constexpr auto array_cast_helper(const std::array<U, i> &a, indices<Is...>) -> std::array<T, i> {
    return {{static_cast<T>(std::get<Is>(a))...}};
  }

  template<typename T, typename U, size_t i>
  constexpr auto array_cast(const std::array<U, i> &a) -> std::array<T, i> {
    // tag dispatch to helper with array indices
    return array_cast_helper<T>(a, build_indices<i>());
  }


  template<size_t N, typename Tin, typename Tout>
  std::vector<std::array<Tout,N>> parse_plydata(PlyData* data) {
    const size_t byte_size = data->buffer.size_bytes();
    const size_t element_size = byte_size/data->count;

    std::vector<std::array<Tout,N>> output; // TODO reserve space
    for (size_t i=0; i<byte_size; i+=element_size) {
      const auto* v = reinterpret_cast<std::array<Tin,N>*>(data->buffer.get()+i);
      output.push_back(array_cast<Tout, Tin, 3>(*v));
    }
    return output;
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

    switch (vertices->t) {
    case tinyply::Type::INT8:
      mesh.vertices = parse_plydata<3, int8_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::UINT8:
      mesh.vertices = parse_plydata<3, uint8_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::INT16:
      mesh.vertices = parse_plydata<3, int16_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::UINT16:
      mesh.vertices = parse_plydata<3, uint16_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::INT32:
      mesh.vertices = parse_plydata<3, int32_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::UINT32:
      mesh.vertices = parse_plydata<3, uint32_t, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::FLOAT64:
      mesh.vertices = parse_plydata<3, double, typename Tmesh::float_t>(vertices.get());
      break;
    case tinyply::Type::FLOAT32:
      mesh.vertices = parse_plydata<3, float, typename Tmesh::float_t>(vertices.get());
      break;
    default:
      throw std::runtime_error("Unsupported vertex type");
    }

    switch (faces->t) {
    case tinyply::Type::INT8:
      mesh.faces = parse_plydata<Tmesh::face_dim, int8_t, typename Tmesh::int_t>(faces.get());
      break;
    case tinyply::Type::UINT8:
      mesh.faces = parse_plydata<Tmesh::face_dim, uint8_t, typename Tmesh::int_t>(faces.get());
      break;
    case tinyply::Type::INT16:
      mesh.faces = parse_plydata<Tmesh::face_dim, int16_t, typename Tmesh::int_t>(faces.get());
      break;
    case tinyply::Type::UINT16:
      mesh.faces = parse_plydata<Tmesh::face_dim, uint16_t, typename Tmesh::int_t>(faces.get());
      break;
    case tinyply::Type::INT32:
      mesh.faces = parse_plydata<Tmesh::face_dim, int32_t, typename Tmesh::int_t>(faces.get());
      break;
    case tinyply::Type::UINT32:
      mesh.faces = parse_plydata<Tmesh::face_dim, uint32_t, typename Tmesh::int_t>(faces.get());
      break;
    default:
      throw std::runtime_error("Unsupported face type");
    }

    return mesh;
  }
}

#endif // PLYIO_HPP
