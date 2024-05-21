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
#include <cmath>
#include <tuple>

// Vertex properties structure
struct VertexProperties {
    float x, y;
    std::string label;
    float size;
    std::string fillcolor;
    int num_solid_edges;
    int required_solid_degree;
    int solid_degree;
    int dashed_degree;
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

std::tuple<SimpleGraph, std::vector<SimpleGraph::vertex_descriptor>> get_initial_graph_and_vertices(int order) {
    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Maximum of vertices
    int max_intermediate_vertices = 2 * (order - 1);

    std::vector<Point> polygon_vertices = calculate_polygon_vertices(max_intermediate_vertices + 2, 1.0);

    // Add a vertex for the initial state
    auto vertex_initial = add_vertex(G);
    vertices.push_back(vertex_initial);
    G[vertex_initial].x = polygon_vertices[0].x;
    G[vertex_initial].y = polygon_vertices[0].y;
    G[vertex_initial].label = "";
    G[vertex_initial].size = 0.5;
    G[vertex_initial].fillcolor = "blue";
    G[vertex_initial].required_solid_degree = 1;
    G[vertex_initial].num_solid_edges = 0;
    G[vertex_initial].solid_degree = 0;
    G[vertex_initial].dashed_degree = 0;

    // Add a vertex for the final state
    auto vertex_final = add_vertex(G);
    vertices.push_back(vertex_final);
    G[vertex_final].x = polygon_vertices[1].x;
    G[vertex_final].y = polygon_vertices[1].y;
    G[vertex_final].label = "";
    G[vertex_final].size = 0.5;
    G[vertex_final].fillcolor = "red";
    G[vertex_final].required_solid_degree = 1;
    G[vertex_final].num_solid_edges = 0;
    G[vertex_final].solid_degree = 0;
    G[vertex_final].dashed_degree = 0;

    // Add intermediate vertices
    for (int i = 0; i < max_intermediate_vertices; ++i) {
        auto v = add_vertex(G);
        vertices.push_back(v);

        // Set random coordinates
        G[v].x = polygon_vertices[i+2].x;
        G[v].y = polygon_vertices[i+2].y;
        G[v].label = "";
        // Set size
        G[v].size = 0.5;
        // Set fillcolor
        G[v].fillcolor = "white";
        // Set number of solid edges
        G[v].required_solid_degree = 2;
        G[v].num_solid_edges = 0;
        G[v].solid_degree = 0;
        G[v].dashed_degree = 0;
    }

    return std::make_tuple(G, vertices);
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

    int max_solid_edges = 2 * order - 1;

    // Create all possible dashed edges
    std::vector<std::pair<int, int>> all_edges;

    int n = 2 * order;

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            all_edges.push_back({i, j});
        }
    }

    // Generate all combinations of order edges
    auto all_dashed_combinations = combinations(all_edges, order);

    for (int num_solid_edges = 0; num_solid_edges < max_solid_edges; ++num_solid_edges) {
        auto all_solid_combinations = combinations(all_edges, num_solid_edges);
        for (const auto& dashed_edges : all_dashed_combinations) {
            for (const auto& solid_edges : all_solid_combinations) {
                // Initialize graph
                std::tuple<SimpleGraph, std::vector<SimpleGraph::vertex_descriptor>> result = get_initial_graph_and_vertices(order);

                SimpleGraph G;
                std::vector<SimpleGraph::vertex_descriptor> vertices;
                
                std::tie(G, vertices) = result;

                // Initialize output flag
                bool output = true;

                // Add dashed edges
                for (const auto& dashed_edge : dashed_edges) {
                    auto e = add_edge(vertices[dashed_edge.first], vertices[dashed_edge.second], G).first;
                    G[vertices[dashed_edge.first]].dashed_degree++;
                    G[vertices[dashed_edge.second]].dashed_degree++;
                    G[e].style = "dashed";
                }

                if (G[vertices[0]].dashed_degree < 1 || G[vertices[1]].dashed_degree < 1) {
                    output = false;
                }

                // Add solid edges
                for (const auto& solid_edge : solid_edges) {
                    auto e = add_edge(vertices[solid_edge.first], vertices[solid_edge.second], G).first;
                    G[vertices[solid_edge.first]].solid_degree++;
                    G[vertices[solid_edge.second]].solid_degree++;
                    G[e].style = "solid";
                }

                if (G[vertices[0]].solid_degree != 1 || G[vertices[1]].solid_degree != 1) {
                    output = false;
                }

                for (const auto& v : vertices) {
                    if (G[v].solid_degree == 0 && G[v].dashed_degree == 0) {
                        G[v].required_solid_degree = 0;
                        remove_vertex(v, G);
                    }
                }

                for (const auto& v : vertices) {
                    if (G[v].solid_degree > 2) {
                        output = false;
                    }
                    if (G[v].dashed_degree == 0 && G[v].solid_degree != 0) {
                        output = false;
                    }
                    if (G[v].dashed_degree != 0 && G[v].solid_degree == 0) {
                        output = false;
                    }
/*
                    if (G[v].solid_degree != G[v].required_solid_degree) {
                        output = false;
                    }
*/
                }

                if (output) {
                    // Labeling
                    for (const auto& v : vertices) {
                        int d = G[v].dashed_degree;
                        G[v].label = std::to_string(d);
                    }

                    // Output
                    std::ofstream file("dot/graph_" + std::to_string(file_counter++) + ".dot");
                    boost::write_graphviz(file, G, vertex_writer(G), edge_writer(G), graph_writer());
                }

                // Remove edges
                for (const auto& dashed_edge : dashed_edges) {
                    remove_edge(vertices[dashed_edge.first], vertices[dashed_edge.second], G);
                }

                for (const auto& solid_edge : solid_edges) {
                    remove_edge(vertices[solid_edge.first], vertices[solid_edge.second], G);
                }
            }
        }
    }

    return 0;
}