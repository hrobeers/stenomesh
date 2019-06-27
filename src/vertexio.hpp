
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

#ifndef VERTEXIO_HPP
#define VERTEXIO_HPP

#include <sstream>
#include <vector>
#include <algorithm>

namespace vertexio
{
  const std::string delimiters = " \t,;";

  inline bool is_float_char(char c)
  {
    if (isdigit(c)) return true;
    const std::string nondigit = "-.eE";
    return std::any_of(nondigit.cbegin(), nondigit.cend(), [c](char f){ return c==f; });
  }

  inline bool is_floats(const std::string &str)
  {
    return std::all_of(str.begin(), str.end(), [](char c){
                                                 if (is_float_char(c)) return true;
                                                 return std::any_of(delimiters.cbegin(), delimiters.cend(), [c](char f){ return c==f; });
                                               });
  }

  template<typename vertex_t>
  std::istream& read_next_vertex(std::istream& str, vertex_t& v)
  {
    // find next line containing floats
    std::string line;
    do
      {
        std::getline(str, line);
      } while(str.peek()!=EOF && !is_floats(line));

    // split line by delimiters
    int i = 0;
    size_t prev = 0;
    while (true)
      {
        auto pos = line.find_first_of(delimiters, prev);
        if (pos > prev)
          std::istringstream(line.substr(prev, pos-prev)) >> v[i++];
        prev = pos+1;
        if (i==v.size() || pos==std::string::npos)
          break;
      }

    return str;
  }
}

#endif // VERTEXIO_HPP
