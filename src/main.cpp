#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <tuple>
#include <vector>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> SimpleGraph;

int main() {
    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Add vertices
    vertices.push_back(add_vertex(G));
    vertices.push_back(add_vertex(G));

    // Add edge
    bool added;
    boost::tie(std::ignore, added) = add_edge(vertices[0], vertices[1], G);

    if (added) {
        std::cout << "Edge added successfully between" << vertices[0] << " and " << vertices[1] << std::endl;
    } else {
        std::cout << "Failed to add edge between" << vertices[0] << " and " << vertices[1] << std::endl;
    }

    // Output graph as Graphviz format (.dot)
    std::ofstream file("graph.dot");
    boost::write_graphviz(file, G);

    return 0;
}