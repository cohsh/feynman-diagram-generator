#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/isomorphism.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <tuple>
#include <queue>
#include <unordered_set>

// Vertex properties structure
struct VertexProperties {
    float x, y;
    std::string label;
    float size;
    std::string fillcolor;
    int required_solid_degree;
    int solid_degree;
    int dashed_degree;
    bool solid_loop;
    bool dashed_loop;
};

// Edge properties structure
struct EdgeProperties {
    std::string style;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperties, EdgeProperties> SimpleGraph;

// Custom vertex property writer
class vertex_writer {
public:
    vertex_writer(const SimpleGraph &g) : g_(g) {}

    template <class Vertex>
    void operator()(std::ostream &out, const Vertex &v) const {
        out << "[label=\"" << g_[v].label << "\", pos=\"" << g_[v].x << "," << g_[v].y << "!\", width=\"" << g_[v].size << "\", height=\"" << g_[v].size << "\", fillcolor=\"" << g_[v].fillcolor << "\", style=filled]";
    }
private:
    const SimpleGraph &g_;
};

// Custom edge property writer
class edge_writer {
public:
    edge_writer(const SimpleGraph &g) : g_(g) {}

    template <class Edge>
    void operator()(std::ostream &out, const Edge &e) const {
        out << "[style=\"" << g_[e].style << "\"]";
    }
private:
    const SimpleGraph &g_;
};

// Custom graph property writer
class graph_writer {
public:
    void operator()(std::ostream &out) const {
        out << "layout=neato;" << std::endl;
    }
};

// Helper function to generate all combinations of k elements from a vector
template <typename T>
std::vector<std::vector<T>> combinations(const std::vector<T>& elements, int k) {
    std::vector<std::vector<T>> result;
    std::vector<int> indices(k, 0);
    int n = elements.size();

    while (true) {
        // Add current combination
        std::vector<T> combination;
        for (int i = 0; i < k; ++i) {
            combination.push_back(elements[indices[i]]);
        }
        result.push_back(combination);
        
        // Move to next combination
        int i = k - 1;
        while (i >= 0 && indices[i] == n - 1) {
            --i;
        }
        
        if (i < 0) {
            break;
        }
        
        ++indices[i];
        for (int j = i + 1; j < k; ++j) {
            indices[j] = indices[i];
        }
    }

    return result;
}

struct Point {
    double x;
    double y;
};

std::vector<Point> calculate_polygon_vertices(int n, double radius) {
    std::vector<Point> vertices;
    // Angle between vertices (radian)
    double angle_increment = 2.0 * M_PI / n;

    for (int i = 0; i < n; ++i) {
        double angle = i * angle_increment;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        vertices.push_back({x, y});
    }

    return vertices;
}

std::tuple<SimpleGraph, std::vector<SimpleGraph::vertex_descriptor>> get_initial_graph_and_vertices(int number_of_vertices) {
    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    std::vector<Point> polygon_vertices = calculate_polygon_vertices(number_of_vertices, 1.0);

    // Add vertices
    for (int i = 0; i < number_of_vertices; ++i) {
        auto v = add_vertex(G);
        vertices.push_back(v);

        // Set coordinates
        G[v].x = polygon_vertices[i].x;
        G[v].y = polygon_vertices[i].y;
        // Set label
        G[v].label = "";
        // Set size
        G[v].size = 0.5;
        // Set fillcolor
        G[v].fillcolor = "white";
        // Set number of solid edges
        G[v].required_solid_degree = 2;
        G[v].solid_degree = 0;
        G[v].dashed_degree = 0;
        // Set having loop flag
        G[v].solid_loop = false;
        G[v].dashed_loop = false;
    }

/*
    // Set properties for the initial and final states
    G[vertices[0]].required_solid_degree = 1;
    G[vertices[0]].fillcolor = "red";

    if (number_of_vertices > 1) {
        G[vertices[1]].required_solid_degree = 1;
        G[vertices[1]].fillcolor = "blue";
    }
*/

    return std::make_tuple(G, vertices);
}

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

bool are_graphs_isomorphic(const SimpleGraph& g1, const SimpleGraph& g2) {
    return boost::isomorphism(g1, g2);
}

int main(int argc, char* argv[]) {
    // argc: Number of command-line args
    // argv: Array of command-line args

    // Ensure the "dot" directory exists
    std::filesystem::create_directories("dot");

    // Counter for output files
    int file_counter = 0;

    // Order of diagrams
    int order = 0;

    if (argc > 1) {
        order = std::atoi(argv[1]);
    } else {
        std::cout << "Order is not specified." << std::endl;
        return 1;
    }

    // Limit of order
    if (order > 3) {
        std::cout << "Please specify the order as 3 or less." << std::endl;
        return 1;
    }

    int max_of_vertices = 2 * order;

    std::vector<SimpleGraph> unique_graphs;

    for (int number_of_vertices = 1; number_of_vertices < max_of_vertices + 1; ++number_of_vertices) {
        // Create all possible edges
        std::vector<std::pair<int, int>> all_edges;

        for (int i = 0; i < number_of_vertices; ++i) {
            for (int j = i; j < number_of_vertices; ++j) {
                all_edges.push_back({i, j});
            }
        }

        // Generate all combinations of order edges
        auto all_dashed_combinations = combinations(all_edges, order);

        for (const auto& dashed_edges : all_dashed_combinations) {
            auto all_solid_combinations = combinations(all_edges, number_of_vertices - 1);

            for (const auto& solid_edges : all_solid_combinations) {
                // Initialize graph
                std::tuple<SimpleGraph, std::vector<SimpleGraph::vertex_descriptor>> result = get_initial_graph_and_vertices(number_of_vertices);

                SimpleGraph G;
                std::vector<SimpleGraph::vertex_descriptor> vertices;
                
                std::tie(G, vertices) = result;

                // Initialize graph_is_correct flag
                bool graph_is_correct = true;

                // Add dashed edges
                for (const auto& dashed_edge : dashed_edges) {
                    auto e = add_edge(vertices[dashed_edge.first], vertices[dashed_edge.second], G).first;
                    G[vertices[dashed_edge.first]].dashed_degree++;
                    G[vertices[dashed_edge.second]].dashed_degree++;
                    if (dashed_edge.first == dashed_edge.second) {
                        G[vertices[dashed_edge.first]].dashed_loop = true;
                    }
                    G[e].style = "dashed";
                }


                // Add solid edges
                for (const auto& solid_edge : solid_edges) {
                    auto e = add_edge(vertices[solid_edge.first], vertices[solid_edge.second], G).first;
                    G[vertices[solid_edge.first]].solid_degree++;
                    G[vertices[solid_edge.second]].solid_degree++;
                    if (solid_edge.first == solid_edge.second) {
                        G[vertices[solid_edge.first]].solid_loop = true;
                    }

                    G[e].style = "solid";
                }

                // Perform DFS to find all vertices connected to vertices[0]
                auto reachable_from_initial = dfs_reachable_vertices(G, vertices[0]);

                for (const auto& v : vertices) {
                    if (reachable_from_initial.find(v) == reachable_from_initial.end()) {
                        graph_is_correct = false;
                        break;
                    }
                }

                if (!graph_is_correct) {
                    continue;
                }

                // Check shape of graph
                int count_initial_and_final_vertices = 0;
                int count_same_initial_and_final_vertices = 0;
                int count_intermediate_vertices = 0;
                int initial_is_found = false;

                for (const auto& v : vertices) {
                    if (G[v].solid_degree == 1) {
                        if (count_initial_and_final_vertices == 0) {
                            G[v].fillcolor = "red";
                        } else if (count_initial_and_final_vertices == 1) {
                            G[v].fillcolor = "blue";
                        }

                        count_initial_and_final_vertices += 1;

                    } else if (G[v].solid_degree == 2) {
                        count_intermediate_vertices += 1;
                    } else if (G[v].solid_degree == 0) {
                        G[v].fillcolor = "red";
                        count_same_initial_and_final_vertices += 1;
                    } else {
                        graph_is_correct = false;
                        break;
                    }

                    if (G[v].dashed_degree == 0) {
                        graph_is_correct = false;
                        break;
                    }

                    if (G[v].solid_loop) {
                        graph_is_correct = false;
                        break;
                    }

                }

                if (!graph_is_correct) {
                    continue;
                }

                if (count_same_initial_and_final_vertices == 0) {
                    if (count_initial_and_final_vertices + count_intermediate_vertices != vertices.size() || count_initial_and_final_vertices > 2) {
                        continue;
                    }
                } else if (count_same_initial_and_final_vertices == 1) {
                    if (count_same_initial_and_final_vertices + count_intermediate_vertices != vertices.size() || count_initial_and_final_vertices != 0) {
                        continue;
                    }
                } else {
                    continue;
                }

                // Labeling
                for (const auto& v : vertices) {
                    int d = G[v].dashed_degree;
                    G[v].label = std::to_string(d);
                }

                bool is_duplicate = false;
                for (const auto& existing_graph : unique_graphs) {
                    if (are_graphs_isomorphic(G, existing_graph)) {
                        is_duplicate = true;
                        break;
                    }
                }

                if (!is_duplicate) {
                    unique_graphs.push_back(G);
                    // write graph
                    std::ofstream file("dot/graph_" + std::to_string(file_counter++) + ".dot");
                    boost::write_graphviz(file, G, vertex_writer(G), edge_writer(G), graph_writer());
                }
            }
        }
    }

    return 0;
}