#include <iostream>
#include <cstdlib>
#include <filesystem>
#include "graph.hpp"
#include "bfs_dfs.hpp"
#include "utility.hpp"

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
    if (order > 2) {
        std::cout << "Please specify the order as 1 or 2." << std::endl;
        return 1;
    }

    int max_of_vertices = 2 * order;

    // flag for ignoring diagrams with fermion-loop
    bool ignore_fermion_loop = true;

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
                            G[v].initial = true;
                        } else if (count_initial_and_final_vertices == 1) {
                            G[v].fillcolor = "blue";
                            G[v].final = true;
                        }

                        count_initial_and_final_vertices += 1;

                    } else if (G[v].solid_degree == 2) {
                        count_intermediate_vertices += 1;
                    } else if (G[v].solid_degree == 0) {
                        G[v].fillcolor = "red";
                        G[v].initial = true;
                        G[v].final = true;
                        count_same_initial_and_final_vertices += 1;
                    } else {
                        graph_is_correct = false;
                        break;
                    }

                    if (G[v].dashed_degree == 0) {
                        graph_is_correct = false;
                        break;
                    }

                    if (G[v].solid_loop && ignore_fermion_loop) {
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
                    // Add short slanted lines to initial and final vertices
                    add_short_slanted_lines(G, vertices);
                    // Align graph
                    // align_vertices(G, vertices, 3.0, 3.0);
                    // Write graph
                    std::ofstream file("dot/graph_" + std::to_string(file_counter++) + ".dot");
                    boost::write_graphviz(file, G, vertex_writer(G), edge_writer(G), graph_writer());
                }
            }
        }
    }

    return 0;
}