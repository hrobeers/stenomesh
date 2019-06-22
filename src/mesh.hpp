
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
  template<size_t N, typename float_t = float, typename idx_t = uint32_t>
  struct Mesh
  {
    std::vector<std::array<float_t,3>> vertices;
    std::vector<std::array<idx_t,N>> faces;
    std::string comment;
  };
}

#endif // MESH_HPP
