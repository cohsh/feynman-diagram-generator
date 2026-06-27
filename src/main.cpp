#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <unordered_set>
#include "graph.hpp"
#include "utility.hpp"

namespace {
using EdgeList = std::vector<std::pair<int, int>>;

// A solid-edge set can only yield a valid diagram if every vertex keeps an
// electron degree <= 2 (the shape check rejects anything else outright), so we
// drop over-degree sets before building a graph for them.
bool solid_degrees_within_bound(const EdgeList& edges, int number_of_vertices) {
    std::vector<int> degree(number_of_vertices, 0);
    for (const auto& e : edges) {
        if (++degree[e.first] > 2 || ++degree[e.second] > 2) {
            return false;
        }
    }
    return true;
}

// A dashed-edge set can only yield a valid diagram if every vertex carries at
// least one phonon line; dashed degree depends solely on the dashed edges, so
// uncovered sets are always rejected and can be skipped early.
bool dashed_covers_all_vertices(const EdgeList& edges, int number_of_vertices) {
    std::vector<bool> covered(number_of_vertices, false);
    for (const auto& e : edges) {
        covered[e.first] = true;
        covered[e.second] = true;
    }
    for (bool c : covered) {
        if (!c) return false;
    }
    return true;
}
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
        std::cout << "Please specify the order as 1, 2, or 3." << std::endl;
        return 1;
    }

    int max_of_vertices = 2 * order;

    // flag for ignoring diagrams with fermion-loop
    bool ignore_fermion_loop = true;

    // Canonical forms of the diagrams emitted so far. A candidate is a duplicate
    // exactly when its canonical form is already present, so dedup is an O(1) hash
    // lookup instead of a pairwise isomorphism scan, and only a short string is
    // kept per diagram rather than the whole graph.
    std::unordered_set<std::string> seen_canonical_forms;

    for (int number_of_vertices = 1; number_of_vertices < max_of_vertices + 1; ++number_of_vertices) {
        // Create all possible edges
        EdgeList all_edges;
        for (int i = 0; i < number_of_vertices; ++i) {
            for (int j = i; j < number_of_vertices; ++j) {
                all_edges.push_back({i, j});
            }
        }

        // Pre-filter the edge sets once per vertex count, keeping only those that
        // can still produce a valid diagram (preserving enumeration order so the
        // surviving candidate sequence is a subsequence of the brute-force one).
        std::vector<EdgeList> dashed_combinations;
        enumerate_combinations(all_edges, order, [&](const EdgeList& combo) {
            if (dashed_covers_all_vertices(combo, number_of_vertices)) {
                dashed_combinations.push_back(combo);
            }
        });

        std::vector<EdgeList> solid_combinations;
        enumerate_combinations(all_edges, number_of_vertices - 1, [&](const EdgeList& combo) {
            if (solid_degrees_within_bound(combo, number_of_vertices)) {
                solid_combinations.push_back(combo);
            }
        });

        for (const auto& dashed_edges : dashed_combinations) {
            for (const auto& solid_edges : solid_combinations) {
                // Initialize graph
                SimpleGraph G;
                std::vector<SimpleGraph::vertex_descriptor> vertices;
                std::tie(G, vertices) = get_initial_graph_and_vertices(number_of_vertices);

                // Add phonon (dashed) and electron (solid) edges
                add_styled_edges(G, vertices, dashed_edges, /*dashed=*/true);
                add_styled_edges(G, vertices, solid_edges, /*dashed=*/false);

                // Keep only connected, well-shaped self-energy diagrams
                if (!is_fully_connected(G, vertices)) {
                    continue;
                }
                if (!classify_and_validate_shape(G, vertices, ignore_fermion_loop)) {
                    continue;
                }

                // Labeling: show each vertex's phonon-line count
                for (const auto& v : vertices) {
                    G[v].label = std::to_string(G[v].dashed_degree);
                }

                // Deduplicate by canonical form (O(1) hash lookup)
                if (seen_canonical_forms.insert(canonical_form(G)).second) {
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