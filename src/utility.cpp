#include "utility.hpp"

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