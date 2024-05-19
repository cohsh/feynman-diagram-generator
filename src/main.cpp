#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>

struct VertexProperties {
    float x, y;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperties> SimpleGraph;

int main() {
    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Number of vertices
    int num_vertices = 10;

    // Set random seed
    std::srand(std::time(0));

    // Add vertices
    for (int i = 0; i < num_vertices; ++i) {
        auto v = add_vertex(G);
        vertices.push_back(v);

        // Set random coordinates
        G[v].x = static_cast<float>(std::rand() % 100) / 10.0f;
        G[v].y = static_cast<float>(std::rand() % 100) / 10.0f;
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
    boost::write_graphviz(file, G, boost::make_label_writer(boost::get(&VertexProperties::x, G)));

    return 0;
}