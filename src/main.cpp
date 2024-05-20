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

// Function to add solid edges recursively
void add_solid_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, const std::vector<std::pair<int, int>>& dashed_edges, std::vector<std::pair<int, int>>& solid_edges, int index, int current_solid, int max_solid_edges, int& file_counter) {
    if (current_solid == max_solid_edges) {
        // Output graph as Graphviz format (.dot)
        std::ofstream file("dot/graph_" + std::to_string(file_counter++) + ".dot");
        boost::write_graphviz(file, G, vertex_writer(G), edge_writer(G), graph_writer());
        return;
    }

    if (index >= dashed_edges.size()) {
        return;
    }

    for (int i = index; i < dashed_edges.size(); ++i) {
        const auto& pair = dashed_edges[i];
        if (G[vertices[pair.first]].num_solid_edges < G[vertices[pair.first]].required_solid_edges &&
            G[vertices[pair.first]].num_solid_edges < G[vertices[pair.first]].required_solid_edges) {
            // Add a solid edge
            auto e = add_edge(vertices[pair.first], vertices[pair.second], G).first;
            G[e].style = "solid";
            G[vertices[pair.first]].num_solid_edges++;
            G[vertices[pair.second]].num_solid_edges++;
            solid_edges.push_back(pair);
            add_solid_edges(G, vertices, dashed_edges, solid_edges, i + 1, current_solid + 1, max_solid_edges, file_counter);
            remove_edge(e, G);
            G[vertices[pair.first]].num_solid_edges--;
            G[vertices[pair.second]].num_solid_edges--;
            solid_edges.pop_back();
        }
    }
}

// Function to add dashed edges recursively
void add_dashed_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, int max_dashed_edges, int index, int current_dashed, std::set<std::pair<int, int>>& existing_edges, std::vector<std::pair<int, int>>& dashed_edges, int& file_counter) {
    if (current_dashed == max_dashed_edges) {
        // After adding dashed edges, connect dashed edge vertices with solid edges in all possible ways
        int max_solid_edges = 2;
        std::vector<std::pair<int, int>> solid_edges;
        add_solid_edges(G, vertices, dashed_edges, solid_edges, 0, 0, max_solid_edges, file_counter);
        return;
    }

    if (index >= vertices.size()) {
        return;
    }

    for (int i = index; i < vertices.size(); ++i) {
        for (int j = 0; j < vertices.size(); ++j) {
            if (existing_edges.find({i, j}) == existing_edges.end() && existing_edges.find({j, i}) == existing_edges.end()) {
                // Add a dashed edge
                auto e = add_edge(vertices[i], vertices[j], G).first;
                G[e].style = "dashed";
                existing_edges.insert({i, j});
                existing_edges.insert({j, i});
                dashed_edges.push_back({i, j});
                add_dashed_edges(G, vertices, max_dashed_edges, i, current_dashed + 1, existing_edges, dashed_edges, file_counter);
                remove_edge(e, G);
                existing_edges.erase({i, j});
                existing_edges.erase({j, i});
                dashed_edges.pop_back();
            }
        }
    }
}

int main() {
    // Ensure the "dot" directory exists
    std::filesystem::create_directories("dot");

    SimpleGraph G;

    // Vector for vertices
    std::vector<SimpleGraph::vertex_descriptor> vertices;

    // Number of vertices
    int num_intermediate_vertices = 1;

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

    // Add a vertex for the final state
    auto vertex_final = add_vertex(G);
    vertices.push_back(vertex_final);
    G[vertex_final].x = 10.0;
    G[vertex_final].y = 0.0;
    G[vertex_final].label = "";
    G[vertex_final].size = 0.5;
    G[vertex_final].fillcolor = "red";
    G[vertex_initial].required_solid_edges = 1;
    G[vertex_final].num_solid_edges = 0;
    G[vertex_final].num_dashed_edges = 0;

    // Add intermediate vertices
    for (int i = 0; i < num_intermediate_vertices; ++i) {
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
    }
    
    // Set about edges
    std::set<std::pair<int, int>> existing_solid_edges;
    std::set<std::pair<int, int>> existing_dashed_edges;

    int num_all_solid_edges = 0;
    for (int i = 0; i < vertices.size(); ++i) {
        num_all_solid_edges += G[vertices[i]].num_solid_edges;
    }
    num_all_solid_edges /= 2;

    // Add edges (all possible combinations)
    std::set<std::pair<int, int>> existing_edges;
    std::vector<std::pair<int, int>> dashed_edges;
    int file_counter = 0;
    int max_dashed_edges = 2;

    add_dashed_edges(G, vertices, max_dashed_edges, 0, 0, existing_edges, dashed_edges, file_counter);

    return 0;
}