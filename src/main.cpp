#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <tuple>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> SimpleGraph;

int main() {
    SimpleGraph G;

    auto vertex_left = add_vertex(G);
    auto vertex_right = add_vertex(G);

    bool added;
    boost::tie(std::ignore, added) = add_edge(vertex_left, vertex_right, G);

    if (added) {
        std::cout << "Edge added successfully between" << vertex_left << " and " << vertex_right << std::endl;
    } else {
        std::cout << "Failed to add edge between" << vertex_left << " and " << vertex_right << std::endl;
    }

    std::ofstream file("graph.dot");
    boost::write_graphviz(file, G);

    return 0;
}