#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <filesystem>

// Vertex properties structure
struct VertexProperties {
    float x, y;
    std::string label;
    float size;
    std::string fillcolor;
    int num_solid_edges;
    int required_solid_edges;
    int num_dashed_edges;
    int degree;
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
    std::vector<bool> v(elements.size());
    std::fill(v.begin(), v.begin() + k, true);

    do {
        std::vector<T> combination;
        for (size_t i = 0; i < elements.size(); ++i) {
            if (v[i]) {
                combination.push_back(elements[i]);
            }
        }
        result.push_back(combination);
    } while (std::prev_permutation(v.begin(), v.end()));

    return result;
}

// Function to add solid edges recursively
void add_solid_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, int max_dashed_edges, int& file_counter) {
    // Create all possible edges
    std::vector<std::pair<int, int>> all_edges;
    int n = vertices.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            all_edges.push_back({i, j});
        }
    }

    for (int i = 0; i < vertices.size() - 1; ++i) {
        auto e_initial = add_edge(vertices[0], vertices[i], G).first;
        G[e_initial].style = "solid";
        for (int j = 0; j < vertices.size() - 1; ++j) {
            auto e_final = add_edge(vertices[1], vertices[j], G).first;
            G[e_final].style = "solid";

            std::cout << std::to_string(i) << " " << std::to_string(j) << std::endl;

            // Generate all combinations of two edges for initial and final states
            auto all_combinations = combinations(all_edges, max_dashed_edges * 2 - 2);

            for (const auto& solid_edges : all_combinations) {
                // Add dashed edges
                for (const auto& edge : solid_edges) {
                    auto e = add_edge(vertices[edge.first], vertices[edge.second], G).first;
                    G[e].style = "solid";
                }
                // Output graph as Graphviz format (.dot)
                std::ofstream file("dot/graph_" + std::to_string(file_counter++) + ".dot");
                boost::write_graphviz(file, G, vertex_writer(G), edge_writer(G), graph_writer());

                        // Remove dashed edges
                for (const auto& edge : solid_edges) {
                    remove_edge(vertices[edge.first], vertices[edge.second], G);
                }
            }
        }
    }

    return;
}

// Function to add dashed edges
void add_dashed_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, int max_dashed_edges, int& file_counter) {
    // Create all possible edges
    std::vector<std::pair<int, int>> all_edges;
    int n = vertices.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            all_edges.push_back({i, j});
        }
    }

    // Generate all combinations of max_dashed_edges edges
    auto all_combinations = combinations(all_edges, max_dashed_edges);

    for (const auto& dashed_edges : all_combinations) {
        // Add dashed edges
        for (int i = 0; i < vertices.size(); ++i) {
            G[vertices[i]].degree = 0;
        }

        for (const auto& edge : dashed_edges) {
            auto e = add_edge(vertices[edge.first], vertices[edge.second], G).first;
            G[vertices[edge.first]].degree++;
            G[vertices[edge.second]].degree++;
            G[e].style = "dashed";
        }

        for (int i = 0; i < vertices.size(); ++i) {
            int d = G[vertices[i]].degree;
            G[vertices[i]].label = std::to_string(d);
        }

        // After adding dashed edges, connect dashed edge vertices with solid edges in all possible ways
        add_solid_edges(G, vertices, max_dashed_edges, file_counter);

        // Remove dashed edges
        for (const auto& edge : dashed_edges) {
            remove_edge(vertices[edge.first], vertices[edge.second], G);
        }
    }

    return;
}

int main() {
    // Ensure the "dot" directory exists
    std::filesystem::create_directories("dot");

    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Counter for output files
    int file_counter = 0;

    // Maximum number of dashed edges
    int max_dashed_edges = 2;

    // Number of vertices
    int max_intermediate_vertices = max_dashed_edges * 2 - 2;

    // Set random seed
    std::srand(static_cast<unsigned>(std::time(0)));

    // Add a vertex for the initial state
    auto vertex_initial = add_vertex(G);
    vertices.push_back(vertex_initial);
    G[vertex_initial].x = 0.0;
    G[vertex_initial].y = 0.0;
    G[vertex_initial].label = "";
    G[vertex_initial].size = 0.5;
    G[vertex_initial].fillcolor = "blue";
    G[vertex_initial].required_solid_edges = 1;
    G[vertex_initial].num_solid_edges = 0;
    G[vertex_initial].num_dashed_edges = 0;
    G[vertex_initial].degree = 0;

    // Add a vertex for the final state
    auto vertex_final = add_vertex(G);
    vertices.push_back(vertex_final);
    G[vertex_final].x = 10.0;
    G[vertex_final].y = 0.0;
    G[vertex_final].label = "";
    G[vertex_final].size = 0.5;
    G[vertex_final].fillcolor = "red";
    G[vertex_final].required_solid_edges = 1;
    G[vertex_final].num_solid_edges = 0;
    G[vertex_final].num_dashed_edges = 0;
    G[vertex_final].degree = 0;

    // Add intermediate vertices
    for (int i = 0; i < max_intermediate_vertices; ++i) {
        auto v = add_vertex(G);
        vertices.push_back(v);

        // Set random coordinates
        G[v].x = static_cast<float>(std::rand() % 100) / 10.0f;
        G[v].y = static_cast<float>(std::rand() % 100) / 10.0f;
        // Set label
//        G[v].label = "$g_{" + std::to_string(i) + "}$";
        G[v].label = "";
        // Set size
        G[v].size = 0.5;
        // Set fillcolor
        G[v].fillcolor = "white";
        // Set number of solid edges
        G[v].required_solid_edges = 2;
        G[v].num_solid_edges = 0;
        G[v].num_dashed_edges = 0;
        G[v].degree = 0;
    }
    
    // Add edges (all possible combinations)
    std::set<std::pair<int, int>> existing_dashed_edges;
    std::vector<std::pair<int, int>> dashed_edges;

    add_dashed_edges(G, vertices, max_dashed_edges, file_counter);

    return 0;
}