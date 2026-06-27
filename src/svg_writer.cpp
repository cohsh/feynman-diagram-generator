#include "svg_writer.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

namespace {

struct Vec { double x, y; };
Vec operator+(Vec a, Vec b) { return {a.x + b.x, a.y + b.y}; }
Vec operator-(Vec a, Vec b) { return {a.x - b.x, a.y - b.y}; }
Vec operator*(Vec a, double s) { return {a.x * s, a.y * s}; }
double len(Vec a) { return std::sqrt(a.x * a.x + a.y * a.y); }
Vec unit(Vec a) { double l = len(a); return l > 1e-9 ? Vec{a.x / l, a.y / l} : Vec{0, 0}; }
Vec perp(Vec a) { return {-a.y, a.x}; }

Vec bezier(const Vec p[4], double t) {
    double u = 1 - t, b0 = u * u * u, b1 = 3 * u * u * t, b2 = 3 * u * t * t, b3 = t * t * t;
    return {b0 * p[0].x + b1 * p[1].x + b2 * p[2].x + b3 * p[3].x,
            b0 * p[0].y + b1 * p[1].y + b2 * p[2].y + b3 * p[3].y};
}
Vec bezier_tangent(const Vec p[4], double t) {
    double u = 1 - t, a0 = 3 * u * u, a1 = 6 * u * t, a2 = 3 * t * t;
    return {a0 * (p[1].x - p[0].x) + a1 * (p[2].x - p[1].x) + a2 * (p[3].x - p[2].x),
            a0 * (p[1].y - p[0].y) + a1 * (p[2].y - p[1].y) + a2 * (p[3].y - p[2].y)};
}

std::string fmt(double v) {
    std::ostringstream o; o.precision(2); o << std::fixed << v; return o.str();
}

// Sample a cubic Bezier into a polyline path; if wavy, ride a sine wave on the
// normal (zero amplitude at the endpoints so it meets vertices cleanly).
std::string curve_path(const Vec p[4], bool wavy, double amp, double wavelength) {
    double L = 0; Vec prev = bezier(p, 0);
    for (int i = 1; i <= 24; ++i) { Vec cur = bezier(p, i / 24.0); L += len(cur - prev); prev = cur; }
    int periods = std::max(2, (int)std::lround(L / wavelength));
    int N = std::max(48, periods * 8);
    std::ostringstream path;
    for (int i = 0; i <= N; ++i) {
        double t = (double)i / N;
        Vec c = bezier(p, t);
        if (wavy) c = c + perp(unit(bezier_tangent(p, t))) * (amp * std::sin(2.0 * M_PI * periods * t));
        path << (i == 0 ? "M " : "L ") << fmt(c.x) << " " << fmt(c.y) << " ";
    }
    return path.str();
}

// A closed wavy circle of radius R centred at O, passing through the touch angle.
std::string wavy_circle(Vec O, double R, double amp, double wavelength, double thetaP) {
    int periods = std::max(8, (int)std::lround(2.0 * M_PI * R / wavelength));
    int N = std::max(96, periods * 10);
    std::ostringstream p;
    for (int i = 0; i <= N; ++i) {
        double theta = thetaP + 2.0 * M_PI * i / N;
        double r = R + amp * std::sin(periods * (theta - thetaP));
        Vec c = O + Vec{std::cos(theta), std::sin(theta)} * r;
        p << (i == 0 ? "M " : "L ") << fmt(c.x) << " " << fmt(c.y) << " ";
    }
    p << "Z";
    return p.str();
}

std::string arrowhead(Vec at, Vec dir, double size, double width) {
    Vec t = unit(dir), n = perp(t);
    Vec tip = at + t * (size * 0.5);
    Vec b1 = at - t * (size * 0.5) + n * width;
    Vec b2 = at - t * (size * 0.5) - n * width;
    std::ostringstream o;
    o << "<path d=\"M " << fmt(tip.x) << " " << fmt(tip.y) << " L " << fmt(b1.x) << " " << fmt(b1.y)
      << " L " << fmt(b2.x) << " " << fmt(b2.y) << " Z\" fill=\"black\"/>\n";
    return o.str();
}

} // namespace

void write_svg(const SimpleGraph& G, const std::string& path) {
    const int V = (int)num_vertices(G);

    struct E { int u, v; bool solid; };
    std::vector<E> edges_v;
    for (auto er = edges(G); er.first != er.second; ++er.first) {
        auto e = *er.first;
        edges_v.push_back({(int)source(e, G), (int)target(e, G), G[e].style == LineStyle::Solid});
    }
    const int M = (int)edges_v.size();

    // --- order vertices along the fermion (electron) line ---
    // Walk the solid subgraph as a trail; this both orders the vertices left to
    // right and orients each electron edge along the fermion flow.
    std::vector<std::vector<std::pair<int, int>>> sinc(V); // (neighbor, edgeIndex)
    std::vector<int> sdeg(V, 0);
    for (int i = 0; i < M; ++i)
        if (edges_v[i].solid) {
            sinc[edges_v[i].u].push_back({edges_v[i].v, i});
            sinc[edges_v[i].v].push_back({edges_v[i].u, i});
            sdeg[edges_v[i].u]++; sdeg[edges_v[i].v]++;
        }
    std::vector<int> orderpos(V, -1);
    std::vector<char> vvis(V, 0), eused(M, 0);
    std::vector<int> a_from(M, -1), a_to(M, -1);
    int next_order = 0;
    auto walk = [&](int s) {
        int cur = s;
        while (cur >= 0) {
            if (!vvis[cur]) { vvis[cur] = 1; orderpos[cur] = next_order++; }
            int chosen = -1, nb = -1;
            for (auto& pr : sinc[cur]) if (!eused[pr.second]) { chosen = pr.second; nb = pr.first; break; }
            if (chosen < 0) break;
            eused[chosen] = 1; a_from[chosen] = cur; a_to[chosen] = nb;
            cur = nb;
        }
    };
    // Prefer to start at the incoming external vertex so flow runs left to right.
    int start = -1;
    for (int v = 0; v < V; ++v) if (sdeg[v] == 1 && G[v].initial) { start = v; break; }
    if (start < 0) for (int v = 0; v < V; ++v) if (sdeg[v] == 1) { start = v; break; }
    if (start < 0) start = 0;
    walk(start);
    for (int v = 0; v < V; ++v) if (!vvis[v]) walk(v);
    for (int i = 0; i < M; ++i) if (edges_v[i].solid && a_from[i] < 0) { a_from[i] = edges_v[i].u; a_to[i] = edges_v[i].v; }

    // --- geometry constants ---
    const double dx = 74, stub = 30, vr = 12, margin = 26;
    const double amp = 4.5, wavelength = 11.0, lw = 1.7;
    const double loop_R = 16, loop_spread = 0.85;

    int cols = std::max(1, next_order);
    auto px = [&](int v) { return margin + stub + orderpos[v] * dx; };

    // group parallel edges (and self-loops) by vertex pair
    std::map<std::pair<int, int>, std::vector<int>> groups;
    for (int i = 0; i < M; ++i)
        groups[{std::min(edges_v[i].u, edges_v[i].v), std::max(edges_v[i].u, edges_v[i].v)}].push_back(i);

    // Decide how to draw each edge. Priority rules:
    //  - a connection between two nodes is drawn straight when possible;
    //  - only one edge per node pair can be straight (others bow into arcs);
    //  - when choosing which edge is the straight one, the electron (solid)
    //    line wins, so phonons bow away from it.
    // A straight line is only sensible for adjacent nodes (span 1); longer
    // connections always arc so they clear the nodes in between.
    enum Kind { STRAIGHT_SOLID, STRAIGHT_PHONON, ARC_ABOVE, ARC_BELOW, SELFLOOP };
    struct Cmd { Kind kind; int e; int lo, hi; double h; int gi, gn; };
    std::vector<Cmd> cmds;
    for (auto& kv : groups) {
        const auto& ids = kv.second;
        int n = (int)ids.size();
        if (edges_v[ids[0]].u == edges_v[ids[0]].v) {
            for (int gi = 0; gi < n; ++gi)
                cmds.push_back({SELFLOOP, ids[gi], edges_v[ids[0]].u, edges_v[ids[0]].u, 0, gi, n});
            continue;
        }
        int u = edges_v[ids[0]].u, v = edges_v[ids[0]].v;
        int lo = orderpos[u] < orderpos[v] ? u : v, hi = (lo == u) ? v : u;
        int span = std::abs(orderpos[u] - orderpos[v]);
        int straight = -1;
        if (span == 1) {                         // only adjacent nodes get a straight edge
            for (int gi = 0; gi < n; ++gi) if (edges_v[ids[gi]].solid) { straight = gi; break; }
            if (straight < 0) straight = 0;      // phonon-only pair: a phonon goes straight
        }
        int aboveStack = 0, belowStack = 0;
        for (int gi = 0; gi < n; ++gi) {
            int i = ids[gi];
            bool solid = edges_v[i].solid;
            if (gi == straight) {
                cmds.push_back({solid ? STRAIGHT_SOLID : STRAIGHT_PHONON, i, lo, hi, 0, 0, 0});
            } else if (solid) {
                double h = 22.0 + 14.0 * (span - 1) + 12.0 * belowStack++;
                cmds.push_back({ARC_BELOW, i, lo, hi, h, 0, 0});
            } else {
                double h = 26.0 + 20.0 * std::max(0, span - 1) + 18.0 * aboveStack++;
                cmds.push_back({ARC_ABOVE, i, lo, hi, h, 0, 0});
            }
        }
    }

    // measure vertical extents to size the canvas
    double above = 2 * loop_R + amp + 6, below = 10;
    for (const auto& c : cmds) {
        if (c.kind == ARC_ABOVE || c.kind == SELFLOOP) above = std::max(above, (c.kind == SELFLOOP ? 2 * loop_R : c.h) + amp + 8);
        if (c.kind == ARC_BELOW) below = std::max(below, c.h + 12);
    }
    double Y0 = margin + above;
    double W = 2 * margin + 2 * stub + (cols - 1) * dx;
    double H = Y0 + below + margin + vr;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << fmt(W) << "\" height=\"" << fmt(H)
        << "\" viewBox=\"0 0 " << fmt(W) << " " << fmt(H) << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    auto P = [&](int v) -> Vec { return {px(v), Y0}; };

    // --- external legs (electron in/out) ---
    auto stub_arrow = [&](int v, bool incoming) {
        Vec p = P(v);
        Vec tip = incoming ? p : p + Vec{stub, 0};
        Vec tail = incoming ? p - Vec{stub, 0} : p;
        svg << "<line x1=\"" << fmt(tail.x) << "\" y1=\"" << fmt(tail.y) << "\" x2=\"" << fmt(tip.x)
            << "\" y2=\"" << fmt(tip.y) << "\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
        Vec mid = (tail + tip) * 0.5;
        svg << arrowhead(mid, {1, 0}, 8, 4.5);
    };
    for (int v = 0; v < V; ++v) if (G[v].initial) stub_arrow(v, true);   // incoming leg
    for (int v = 0; v < V; ++v) if (G[v].final) stub_arrow(v, false);    // outgoing leg

    // --- edges ---
    auto solid_arrow_on_curve = [&](const Vec ctrl[4], int i) {
        Vec m = bezier(ctrl, 0.5), tan = unit(bezier_tangent(ctrl, 0.5));
        Vec flow = unit(P(a_to[i]) - P(a_from[i]));
        if (tan.x * flow.x + tan.y * flow.y < 0) tan = tan * -1.0;
        svg << arrowhead(m, tan, 8.5, 5);
    };
    for (const auto& c : cmds) {
        if (c.kind == SELFLOOP) {
            Vec p = P(c.lo);
            double ang = (c.gi - (c.gn - 1) / 2.0) * loop_spread;
            Vec dir = {std::cos(-M_PI / 2 + ang), std::sin(-M_PI / 2 + ang)};
            Vec O = p + dir * loop_R;
            double thetaP = std::atan2(-dir.y, -dir.x);
            svg << "<path d=\"" << wavy_circle(O, loop_R, amp * 0.7, wavelength, thetaP)
                << "\" fill=\"none\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
            continue;
        }
        Vec A = P(c.lo), B = P(c.hi);
        if (c.kind == STRAIGHT_SOLID) {
            svg << "<line x1=\"" << fmt(A.x) << "\" y1=\"" << fmt(A.y) << "\" x2=\"" << fmt(B.x)
                << "\" y2=\"" << fmt(B.y) << "\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
            Vec flow = unit(P(a_to[c.e]) - P(a_from[c.e]));
            svg << arrowhead((A + B) * 0.5, flow, 8.5, 5);
        } else if (c.kind == STRAIGHT_PHONON) {
            Vec ctrl[4] = {A, A + (B - A) * (1.0 / 3.0), A + (B - A) * (2.0 / 3.0), B};
            svg << "<path d=\"" << curve_path(ctrl, true, amp, wavelength)
                << "\" fill=\"none\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
        } else if (c.kind == ARC_ABOVE) {
            Vec ctrl[4] = {A, {A.x + (B.x - A.x) * 0.15, Y0 - c.h * 1.30},
                           {B.x - (B.x - A.x) * 0.15, Y0 - c.h * 1.30}, B};
            svg << "<path d=\"" << curve_path(ctrl, true, amp, wavelength)
                << "\" fill=\"none\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
        } else { // ARC_BELOW (extra electron line, e.g. a fermion loop)
            Vec ctrl[4] = {A, {A.x + (B.x - A.x) * 0.2, Y0 + c.h * 1.30},
                           {B.x - (B.x - A.x) * 0.2, Y0 + c.h * 1.30}, B};
            svg << "<path d=\"" << curve_path(ctrl, false, 0, wavelength)
                << "\" fill=\"none\" stroke=\"black\" stroke-width=\"" << fmt(lw) << "\"/>\n";
            solid_arrow_on_curve(ctrl, c.e);
        }
    }

    // --- vertices ---
    for (int v = 0; v < V; ++v) {
        Vec p = P(v);
        std::string fill = G[v].fillcolor.empty() ? "white" : G[v].fillcolor;
        svg << "<circle cx=\"" << fmt(p.x) << "\" cy=\"" << fmt(p.y) << "\" r=\"" << fmt(vr)
            << "\" fill=\"" << fill << "\" stroke=\"black\" stroke-width=\"1.3\"/>\n";
        if (!G[v].label.empty()) {
            std::string tc = (fill == "white") ? "black" : "white";
            svg << "<text x=\"" << fmt(p.x) << "\" y=\"" << fmt(p.y + 4)
                << "\" font-size=\"13\" font-family=\"sans-serif\" text-anchor=\"middle\" fill=\""
                << tc << "\">" << G[v].label << "</text>\n";
        }
    }

    svg << "</svg>\n";
    std::ofstream f(path);
    f << svg.str();
}
