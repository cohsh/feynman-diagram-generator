#!/bin/sh

generate_diagram() {
    ./build/feynman_diagram_generator
}

output_type=$1

if [ "$output_type" = "png" ]; then
    generate_diagram
    dot -Tpng graph.dot -o graph.png
elif [ "$output_type" = "tex" ]; then
    generate_diagram
    dot2tex -tmath graph.dot > graph.tex
    pdflatex graph.tex
else
    echo "Error: Invalid output type. Please specify 'png' or 'tex'."
    exit 1
fi