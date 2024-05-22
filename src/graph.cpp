#include "graph.hpp"
#include "utility.hpp"
#include "bfs_dfs.hpp"

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
        // Set initial and final flags
        G[v].initial = false;
        G[v].final = false;
    }

    return std::make_tuple(G, vertices);
}

bool are_graphs_isomorphic(const SimpleGraph& g1, const SimpleGraph& g2) {
    return boost::isomorphism(g1, g2);
}

void align_vertices(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices, double x_length, double y_length) {
    std::vector<SimpleGraph::vertex_descriptor> lower_vertices;
    std::vector<SimpleGraph::vertex_descriptor> upper_vertices;

    SimpleGraph::vertex_descriptor v_initial;
    SimpleGraph::vertex_descriptor v_final;

    bool initial_is_final = false;
    // Search initial and final vertices
    for (const auto& v : vertices) {
        if (G[v].initial && !G[v].final) {
            v_initial = v;
            lower_vertices.push_back(v_initial);
        }
        if (G[v].final && !G[v].initial) {
            v_final = v;
        }

        if (G[v].final && G[v].initial) {
            initial_is_final = true;
            v_initial = v;
            v_final = v;
            lower_vertices.push_back(v_initial);
        }
    }

    // Perform DFS to find all vertices connected to vertices[0]
    auto reachable_from_initial = dfs_reachable_vertices_solid(G, v_initial);

    for (const auto& v : vertices) {
        if (!G[v].initial && !G[v].final) {
            if (reachable_from_initial.find(v) != reachable_from_initial.end()) {
                lower_vertices.push_back(v);
            } else {
                upper_vertices.push_back(v);
            }
        }
    }

    // Add final vertex
    if (!initial_is_final) {
        lower_vertices.push_back(v_final);
    }

    // Align lower
    if (lower_vertices.size() > 1) {
        double dx_lower = x_length / (lower_vertices.size() - 1);
        // Align
        for (int i = 0; i < lower_vertices.size(); ++i) {
            G[lower_vertices[i]].x = dx_lower * i;
            G[lower_vertices[i]].y = 0.0;
        }

    } else {
        G[lower_vertices[0]].x = 0.0;
        G[lower_vertices[0]].y = 0.0;
    }

    // Align upper
    if (upper_vertices.size() > 1) {
        double dx_upper = x_length / (upper_vertices.size() - 1);
        // Align
        for (int i = 0; i < upper_vertices.size(); ++i) {
            G[upper_vertices[i]].x = dx_upper * i;
            G[upper_vertices[i]].y = y_length;
        }
    } else if (upper_vertices.size() == 1) {
        G[upper_vertices[0]].x = 0.0;
        G[upper_vertices[0]].y = y_length;
    }

    return;
}

void add_short_slanted_lines(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices) {
    double length = 0.3;
    SimpleGraph::vertex_descriptor v_initial;
    SimpleGraph::vertex_descriptor v_final;
    double sign = 1.0;

    for (const auto& v : vertices) {
        if (G[v].initial && !G[v].final) {
            v_initial = v;
        }
        if (G[v].final && !G[v].initial) {
            v_final = v;
        }

        if (G[v].final && G[v].initial) {
            v_initial = v;
            v_final = v;
        }
    }

    if (G[v_initial].x > G[v_final].x) {
        sign = -1.0;
    }

    for (const auto& v : vertices) {
        if (G[v].initial) {
            // Add a dummy vertex
            auto dummy = add_vertex(G);
            // Position the dummy vertex slightly offset from the original vertex
            G[dummy].x = G[v].x - sign * length; // Adjust as needed for desired slant length
            G[dummy].y = G[v].y - length; // Adjust as needed for desired slant length
            G[dummy].label = "";
            G[dummy].size = 0.0;
            G[dummy].fillcolor = "white";

            // Add an edge between the original vertex and the dummy vertex
            auto e = add_edge(dummy, v, G).first;
            G[e].style = "solid";
        }

        if (G[v].final) {
            // Add a dummy vertex
            auto dummy = add_vertex(G);
            // Position the dummy vertex slightly offset from the original vertex
            G[dummy].x = G[v].x + sign * length; // Adjust as needed for desired slant length
            G[dummy].y = G[v].y - length; // Adjust as needed for desired slant length
            G[dummy].label = "";
            G[dummy].size = 0.0;
            G[dummy].fillcolor = "white";

            // Add an edge between the original vertex and the dummy vertex
            auto e = add_edge(v, dummy, G).first;
            G[e].style = "solid";
        }

    }
}