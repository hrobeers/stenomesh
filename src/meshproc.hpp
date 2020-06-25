
#ifndef MESHPROC_HPP
#define MESHPROC_HPP

#include <map>
#include <algorithm>
#include "mesh.hpp"

namespace stenomesh {
  template<typename Tarr>
  bool all_distinct(Tarr arr) {
    std::sort(std::begin(arr), std::end(arr));
    auto pos = std::adjacent_find(std::begin(arr), std::end(arr));
    return (pos == std::end(arr));
  }

  template<typename Tarr_in, typename Tarr_out>
  Tarr_out multiply(const Tarr_in &array, double factor) {
    Tarr_out out;
    for (size_t i=0; i<array.size(); i++)
      out[i] = (typename Tarr_out::value_type)(array[i] * factor);
    return out;
  }

  template<typename vertices_t, typename dup_map_t>
  uint32_t dedup_insert(typename vertices_t::value_type new_vertex, vertices_t &vertices , dup_map_t &duplicates, double fprec) {
    auto va = multiply<typename vertices_t::value_type, typename dup_map_t::key_type>(new_vertex, fprec);
    auto dup_search = duplicates.find(va);
    if (dup_search != duplicates.end())
      return dup_search->second;

    uint32_t new_idx = vertices.size();
    duplicates[va] = new_idx;
    vertices.push_back(new_vertex);
    return new_idx;
  }

  template<typename TMesh>
  void vertex_merge(TMesh &mesh, double dist = 1/1e2) {
    std::map<std::array<ssize_t,3>, typename TMesh::idx_t> merge_map;

    typename TMesh::faces_t new_faces;
    typename TMesh::vertices_t new_vertices;
    for (auto face : mesh.faces) {
      typename TMesh::faces_t::value_type new_face;
      size_t i = 0;
      for (auto vtx_idx : face)
        new_face[i++] = dedup_insert(mesh.vertices[vtx_idx], new_vertices, merge_map, 1/dist);
      if (all_distinct(new_face))
        new_faces.push_back(new_face);
    }
    mesh.faces.swap(new_faces);
    mesh.vertices.swap(new_vertices);
  }

  template<typename TMesh>
  std::array<typename TMesh::vertices_t::value_type, 2> bounding_box(const TMesh &mesh) {
    std::array<typename TMesh::vertices_t::value_type, 2> bbox =
      { mesh.vertices.front(), mesh.vertices.front() };
    for (auto vtx : mesh.vertices)
      for (size_t i=0;i<vtx.size();i++) {
        bbox[0][i] = std::min(bbox[0][i],vtx[i]);
        bbox[1][i] = std::max(bbox[1][i],vtx[i]);
      }
    return bbox;
  }
}

#endif // MESHPROC_HPP
