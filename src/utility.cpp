#include "utility.hpp"

template <typename T>
std::vector<std::vector<T>> combinations(const std::vector<T>& elements, int k) {
    std::vector<std::vector<T>> result;
    std::vector<int> indices(k, 0);
    int n = elements.size();

    while (true) {
        // Add current combination
        std::vector<T> combination;
        for (int i = 0; i < k; ++i) {
            combination.push_back(elements[indices[i]]);
        }
        result.push_back(combination);
        
        // Move to next combination
        int i = k - 1;
        while (i >= 0 && indices[i] == n - 1) {
            --i;
        }
        
        if (i < 0) {
            break;
        }
        
        ++indices[i];
        for (int j = i + 1; j < k; ++j) {
            indices[j] = indices[i];
        }
    }

    return result;
}

std::vector<Point> calculate_polygon_vertices(int n, double radius) {
    std::vector<Point> vertices;
    // Angle between vertices (radian)
    double angle_increment = 2.0 * M_PI / n;

    for (int i = 0; i < n; ++i) {
        double angle = i * angle_increment;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        vertices.push_back({x, y});
    }

    return vertices;
}

template std::vector<std::vector<int>> combinations(const std::vector<int>& elements, int k);
template std::vector<std::vector<std::pair<int, int>>> combinations(const std::vector<std::pair<int, int>>& elements, int k);