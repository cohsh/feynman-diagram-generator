#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>

// Vertex properties structure
struct VertexProperties {
    float x, y;
    std::string label;
    float size;
    std::string fillcolor;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperties> SimpleGraph;

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

// Custom graph property writer
class graph_writer {
public:
    void operator()(std::ostream &out) const {
        out << "layout=neato;" << std::endl;
    }
};

int main() {
    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Number of vertices
    int num_vertices = 10;

    // Set random seed
    std::srand(static_cast<unsigned>(std::time(0)));

    // Add a vertex for the initial state
    auto vertex_initial = add_vertex(G);
    vertices.push_back(vertex_initial);
    G[vertex_initial].x = 0.0;
    G[vertex_initial].y = 0.0;
    G[vertex_initial].label = "$v_{i}$";
    G[vertex_initial].size = 0.7;
    G[vertex_initial].fillcolor = "blue";

    // Add a vertex for the final state
    auto vertex_final = add_vertex(G);
    vertices.push_back(vertex_final);
    G[vertex_final].x = 10.0;
    G[vertex_final].y = 0.0;
    G[vertex_final].label = "$v_{f}$";
    G[vertex_final].size = 0.7;
    G[vertex_final].fillcolor = "red";

    // Add vertices
    for (int i = 0; i < num_vertices; ++i) {
        auto v = add_vertex(G);
        vertices.push_back(v);

        // Set random coordinates
        G[v].x = static_cast<float>(std::rand() % 100) / 10.0f;
        G[v].y = static_cast<float>(std::rand() % 100) / 10.0f;
        // Set label
        G[v].label = "$v_{" + std::to_string(i) + "}$";
        // Set size
        G[v].size = 0.7;
        G[v].fillcolor = "white";
    }

    // Add edge
    for (int i = 0; i < num_vertices - 1; ++i) {
        int source = std::rand() % num_vertices;
        int target = std::rand() % num_vertices;

        bool added;
        boost::tie(std::ignore, added) = add_edge(vertices[source], vertices[target], G);

        if (added) {
            std::cout << "Edge added successfully between" << source << " and " << target << std::endl;
        } else {
            std::cout << "Failed to add edge between" << source << " and " << target << std::endl;
        }
    }

    // Output graph as Graphviz format (.dot)
    std::ofstream file("graph.dot");
    boost::write_graphviz(file, G, vertex_writer(G), boost::default_writer(), graph_writer());

    return 0;
}