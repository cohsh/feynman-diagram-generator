#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <cmath>

namespace detail {
template <typename T, typename F>
void enumerate_combinations_rec(const std::vector<T>& elements, int k, int start,
                                std::vector<T>& current, F& visit) {
    if (static_cast<int>(current.size()) == k) {
        visit(current);
        return;
    }
    for (int i = start; i < static_cast<int>(elements.size()); ++i) {
        current.push_back(elements[i]);
        enumerate_combinations_rec(elements, k, i, current, visit);
        current.pop_back();
    }
}
}

// Stream every size-k combination with repetition of `elements`, in the same
// lexicographic (non-decreasing index) order as a fully materialized list, but
// without building that list. `visit` is called with each combination in turn,
// letting the caller filter on the fly instead of allocating all of them.
template <typename T, typename F>
void enumerate_combinations(const std::vector<T>& elements, int k, F visit) {
    std::vector<T> current;
    if (k > 0) current.reserve(k);
    detail::enumerate_combinations_rec(elements, k, 0, current, visit);
}

struct Point {
    double x;
    double y;
};

std::vector<Point> calculate_polygon_vertices(int n, double radius);

#endif
