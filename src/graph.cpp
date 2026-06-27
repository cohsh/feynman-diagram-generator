#include "graph.hpp"
#include "utility.hpp"
#include "bfs_dfs.hpp"
#include <algorithm>
#include <map>
#include <array>
#include <functional>

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
        // Set edge degree counters
        G[v].solid_degree = 0;
        G[v].dashed_degree = 0;
        // Set having loop flag
        G[v].solid_loop = false;
        // Set initial and final flags
        G[v].initial = false;
        G[v].final = false;
    }

    return std::make_tuple(G, vertices);
}

void add_styled_edges(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices,
                      const std::vector<std::pair<int, int>>& edges, bool dashed) {
    for (const auto& edge : edges) {
        auto e = add_edge(vertices[edge.first], vertices[edge.second], G).first;
        if (dashed) {
            G[vertices[edge.first]].dashed_degree++;
            G[vertices[edge.second]].dashed_degree++;
            G[e].style = LineStyle::Dashed;
        } else {
            G[vertices[edge.first]].solid_degree++;
            G[vertices[edge.second]].solid_degree++;
            if (edge.first == edge.second) {
                G[vertices[edge.first]].solid_loop = true;
            }
            G[e].style = LineStyle::Solid;
        }
    }
}

std::string canonical_form(const SimpleGraph& G) {
    const int V = static_cast<int>(num_vertices(G));

    // Collect edges as (endpoint_a, endpoint_b, style) and build a colored
    // incidence list. Each edge contributes to both endpoints (a self-loop
    // therefore appears twice on its vertex, matching the degree counters).
    std::vector<std::array<int, 3>> edge_list;
    std::vector<std::vector<std::pair<int, int>>> incidence(V); // (neighbor, style)
    for (auto er = edges(G); er.first != er.second; ++er.first) {
        const auto e = *er.first;
        const int a = static_cast<int>(source(e, G));
        const int b = static_cast<int>(target(e, G));
        const int style = (G[e].style == LineStyle::Solid) ? 0 : 1;
        edge_list.push_back({a, b, style});
        incidence[a].push_back({b, style});
        incidence[b].push_back({a, style});
    }

    // 1-WL colour refinement. The initial colour encodes (solid, dashed) degree;
    // each round replaces it by a colour derived from the multiset of coloured
    // neighbours. Colour ids are assigned by the sorted rank of their signature
    // (NOT by first-appearance order) so that isomorphic graphs, whose vertices
    // may be numbered differently, receive identical colourings. Colours are
    // isomorphism-invariant, so the resulting cells only constrain (never break)
    // the canonical search below.
    auto rank_signatures = [V](const std::vector<std::vector<int>>& sigs, std::vector<int>& out) {
        std::map<std::vector<int>, int> rank;
        for (const auto& s : sigs) rank.emplace(s, 0);
        int r = 0;
        for (auto& kv : rank) kv.second = r++;
        out.resize(V);
        for (int v = 0; v < V; ++v) out[v] = rank.at(sigs[v]);
        return static_cast<int>(rank.size());
    };

    std::vector<int> color;
    {
        std::vector<std::vector<int>> base(V);
        for (int v = 0; v < V; ++v) base[v] = {G[v].solid_degree, G[v].dashed_degree};
        rank_signatures(base, color);
    }
    int distinct = 0;
    for (int v = 0; v < V; ++v) distinct = std::max(distinct, color[v] + 1);
    while (true) {
        std::vector<std::vector<int>> sigs(V);
        for (int v = 0; v < V; ++v) {
            std::vector<std::pair<int, int>> nb;
            nb.reserve(incidence[v].size());
            for (const auto& pr : incidence[v]) nb.push_back({color[pr.first], pr.second});
            std::sort(nb.begin(), nb.end());
            std::vector<int>& sig = sigs[v];
            sig.reserve(1 + 2 * nb.size());
            sig.push_back(color[v]);
            for (const auto& pr : nb) { sig.push_back(pr.first); sig.push_back(pr.second); }
        }
        std::vector<int> next;
        int new_distinct = rank_signatures(sigs, next);
        if (new_distinct == distinct) break; // partition is stable
        color = std::move(next);
        distinct = new_distinct;
    }

    // Group vertices into cells ordered by their stable colour. The canonical
    // labelling numbers vertices cell by cell; an isomorphism must respect cells,
    // so we only permute within each cell and keep the lexicographically smallest
    // sorted edge list. Equal canonical strings <=> isomorphic graphs.
    std::map<int, std::vector<int>> cell_map;
    for (int v = 0; v < V; ++v) cell_map[color[v]].push_back(v);
    std::vector<std::vector<int>> cells;
    for (auto& kv : cell_map) cells.push_back(std::move(kv.second));

    std::vector<int> perm(V);
    std::string best;
    std::function<void(int, int)> search = [&](int ci, int base) {
        if (ci == static_cast<int>(cells.size())) {
            std::vector<std::array<int, 3>> relabeled;
            relabeled.reserve(edge_list.size());
            for (const auto& e : edge_list) {
                int u = perm[e[0]], w = perm[e[1]];
                if (u > w) std::swap(u, w);
                relabeled.push_back({u, w, e[2]});
            }
            std::sort(relabeled.begin(), relabeled.end());
            std::string key;
            for (const auto& e : relabeled) {
                key += std::to_string(e[0]);
                key += ',';
                key += std::to_string(e[1]);
                key += ',';
                key += std::to_string(e[2]);
                key += ';';
            }
            if (best.empty() || key < best) best = std::move(key);
            return;
        }
        std::vector<int> members = cells[ci];
        std::sort(members.begin(), members.end());
        do {
            for (int i = 0; i < static_cast<int>(members.size()); ++i) {
                perm[members[i]] = base + i;
            }
            search(ci + 1, base + static_cast<int>(members.size()));
        } while (std::next_permutation(members.begin(), members.end()));
    };
    search(0, 0);

    return std::to_string(V) + "#" + best;
}

bool is_fully_connected(const SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices) {
    auto reachable = dfs_reachable_vertices(G, vertices[0]);
    for (const auto& v : vertices) {
        if (reachable.find(v) == reachable.end()) {
            return false;
        }
    }
    return true;
}

bool classify_and_validate_shape(SimpleGraph& G, const std::vector<SimpleGraph::vertex_descriptor>& vertices,
                                 bool ignore_fermion_loop) {
    int count_initial_and_final_vertices = 0;
    int count_same_initial_and_final_vertices = 0;
    int count_intermediate_vertices = 0;

    for (const auto& v : vertices) {
        if (G[v].solid_degree == 1) {
            // A vertex with a single electron line is an external (initial/final) vertex.
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
            // No electron line: initial and final coincide on this vertex.
            G[v].fillcolor = "red";
            G[v].initial = true;
            G[v].final = true;
            count_same_initial_and_final_vertices += 1;
        } else {
            return false;
        }

        // Every vertex must carry at least one phonon line.
        if (G[v].dashed_degree == 0) {
            return false;
        }

        if (G[v].solid_loop && ignore_fermion_loop) {
            return false;
        }
    }

    if (count_same_initial_and_final_vertices == 0) {
        if (count_initial_and_final_vertices + count_intermediate_vertices != (int)vertices.size() || count_initial_and_final_vertices > 2) {
            return false;
        }
    } else if (count_same_initial_and_final_vertices == 1) {
        if (count_same_initial_and_final_vertices + count_intermediate_vertices != (int)vertices.size() || count_initial_and_final_vertices != 0) {
            return false;
        }
    } else {
        return false;
    }

    return true;
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
        for (std::size_t i = 0; i < lower_vertices.size(); ++i) {
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
        for (std::size_t i = 0; i < upper_vertices.size(); ++i) {
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
            G[e].style = LineStyle::Solid;
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
            G[e].style = LineStyle::Solid;
        }

    }
}