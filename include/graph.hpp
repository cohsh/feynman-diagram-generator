#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>
#include <vector>
#include <tuple>

// Vertex properties structure
struct VertexProperties {
    float x, y;
    std::string label;
    float size;
    std::string fillcolor;
    int solid_degree;
    int dashed_degree;
    bool solid_loop;
    bool initial;
    bool final;
};

// Line type carried by an edge: electron (solid) or phonon (dashed).
enum class LineStyle { Solid, Dashed };

inline const char* to_dot_style(LineStyle style) {
    return style == LineStyle::Dashed ? "dashed" : "solid";
}

// Edge properties structure
struct EdgeProperties {
    LineStyle style;
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
        out << "[style=\"" << to_dot_style(g_[e].style) << "\"]";
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

std::tuple<SimpleGraph, std::vector<SimpleGraph::vertex_descriptor>> get_initial_graph_and_vertices(int number_of_vertices);
void align_vertices(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, double x_length, double y_length);
void add_short_slanted_lines(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices);

// Add a set of edges of one style (dashed = phonon, solid = electron) to G,
// updating the per-vertex degree counters and the fermion-loop flag.
void add_styled_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices,
                      const std::vector<std::pair<int, int>>& edges, bool dashed);
// True if every vertex is reachable from vertices[0] over all edges.
bool is_fully_connected(const SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices);
// A canonical string for the edge-coloured (electron/phonon) multigraph: two
// diagrams are isomorphic if and only if their canonical forms are equal. This
// lets deduplication use a hash set instead of pairwise isomorphism tests.
std::string canonical_form(const SimpleGraph& G);
// Classify vertices (initial/final/intermediate) by their solid degree, colour
// them, and decide whether the graph is a valid self-energy diagram. Mutates G.
bool classify_and_validate_shape(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices,
                                 bool ignore_fermion_loop);
// True if the diagram is proper (one-particle-irreducible): no internal electron
// line is a bridge, i.e. cutting any single electron propagator leaves the graph
// connected. Improper (reducible) diagrams are resummed by the Dyson equation and
// so are normally excluded from the self-energy.
bool is_proper_diagram(const SimpleGraph& G);

#endif