#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <cmath>

// Helper function to generate all combinations of k elements from a vector
template <typename T>
std::vector<std::vector<T>> combinations(const std::vector<T>& elements, int k);

struct Point {
    double x;
    double y;
};

std::vector<Point> calculate_polygon_vertices(int n, double radius);

#endif