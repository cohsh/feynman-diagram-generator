#ifndef BFS_DFS_HPP
#define BFS_DFS_HPP

#include "graph.hpp"
#include <unordered_set>
#include <stack>

// Reachability via all edges (used for the global connectivity check).
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
// Reachability via solid (electron) edges only (used when aligning vertices).
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices_solid(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);

#endif