#include "bfs_dfs.hpp"

// BFS to find all vertices reachable from the start vertex
std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::queue<SimpleGraph::vertex_descriptor> q;

    visited.insert(start);
    q.push(start);

    while (!q.empty()) {
        auto u = q.front();
        q.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            auto v = target(*edge_it.first, G);
            if (visited.find(v) == visited.end()) {
                visited.insert(v);
                q.push(v);
            }
        }
    }

    return visited;
}

// BFS to find all vertices reachable from the start vertex via solid edges
std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices_solid(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::queue<SimpleGraph::vertex_descriptor> q;

    visited.insert(start);
    q.push(start);

    while (!q.empty()) {
        auto u = q.front();
        q.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            if (G[*edge_it.first].style == "solid") {
                auto v = target(*edge_it.first, G);
                if (visited.find(v) == visited.end()) {
                    visited.insert(v);
                    q.push(v);
                }
            }
        }
    }

    return visited;
}

// BFS to find all vertices reachable from the start vertex via dashed edges
std::unordered_set<SimpleGraph::vertex_descriptor> bfs_reachable_vertices_dashed(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::queue<SimpleGraph::vertex_descriptor> q;

    visited.insert(start);
    q.push(start);

    while (!q.empty()) {
        auto u = q.front();
        q.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            if (G[*edge_it.first].style == "dashed") {
                auto v = target(*edge_it.first, G);
                if (visited.find(v) == visited.end()) {
                    visited.insert(v);
                    q.push(v);
                }
            }
        }
    }

    return visited;
}

// DFS to find all vertices reachable from the start vertex
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::stack<SimpleGraph::vertex_descriptor> s;

    visited.insert(start);
    s.push(start);

    while (!s.empty()) {
        auto u = s.top();
        s.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            auto v = target(*edge_it.first, G);
            if (visited.find(v) == visited.end()) {
                visited.insert(v);
                s.push(v);
            }
        }
    }

    return visited;
}

// DFS to find all vertices reachable from the start vertex via solid edges
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices_solid(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::stack<SimpleGraph::vertex_descriptor> s;

    visited.insert(start);
    s.push(start);

    while (!s.empty()) {
        auto u = s.top();
        s.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            if (G[*edge_it.first].style == "solid") {
                auto v = target(*edge_it.first, G);
                if (visited.find(v) == visited.end()) {
                    visited.insert(v);
                    s.push(v);
                }
            }
        }
    }

    return visited;
}

// DFS to find all vertices reachable from the start vertex via dashed edges
std::unordered_set<SimpleGraph::vertex_descriptor> dfs_reachable_vertices_dashed(const SimpleGraph &G, SimpleGraph::vertex_descriptor start) {
    std::unordered_set<SimpleGraph::vertex_descriptor> visited;
    std::stack<SimpleGraph::vertex_descriptor> s;

    visited.insert(start);
    s.push(start);

    while (!s.empty()) {
        auto u = s.top();
        s.pop();

        for (auto edge_it = out_edges(u, G); edge_it.first != edge_it.second; ++edge_it.first) {
            if (G[*edge_it.first].style == "dashed") {
                auto v = target(*edge_it.first, G);
                if (visited.find(v) == visited.end()) {
                    visited.insert(v);
                    s.push(v);
                }
            }
        }
    }

    return visited;
}