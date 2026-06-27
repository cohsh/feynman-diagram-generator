#ifndef SVG_WRITER_HPP
#define SVG_WRITER_HPP

#include "graph.hpp"
#include <string>

// Render the diagram G to a standalone SVG file (no external tool needed).
// Electron lines are drawn solid with a flow arrow; phonon lines are drawn as
// wavy lines; vertices are coloured circles (red = initial, blue = final).
void write_svg(const SimpleGraph& G, const std::string& path);

#endif
