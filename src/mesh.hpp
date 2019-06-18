
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

#ifndef MESH_HPP
#define MESH_HPP

#include <array>
#include <vector>
#include <unistd.h>

namespace stenomesh {
  template<size_t N, typename Tflt = float, typename Tint = uint32_t>
  struct Mesh
  {
    const static size_t face_dim = N;

    typedef Tflt float_t;
    typedef Tint int_t;

    template <typename T = float_t>
    using vertex_t = std::array<T,3>;

    template <typename T = int_t>
    using face_t = std::array<T, face_dim>;

    typedef std::vector<vertex_t<>> vertices_t;
    typedef std::vector<face_t<>> faces_t;

    vertices_t vertices;
    faces_t faces;
    std::vector<std::string> comments;
  };
}

#endif // MESH_HPP
