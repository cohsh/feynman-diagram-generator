#ifndef BFS_DFS_HPP
#define BFS_DFS_HPP

#include "graph.hpp"
#include <unordered_set>
#include <queue>
#include <stack>

std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices_solid(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices_dashed(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices_solid(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices_dashed(const SimpleGraph &G, SimpleGraph::vertex_descriptor start);

#endif