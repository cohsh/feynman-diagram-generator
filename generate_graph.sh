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
    dot2tex -t raw --preproc graph.dot > pre.tex
#    dot2tex -t raw -f tikz --autosize --crop --figpreamble="\huge" pre.tex > graph.tex
    dot2tex -t raw -f tikz --autosize --crop pre.tex > graph.tex
    pdflatex graph.tex
else
    echo "Error: Invalid output type. Please specify 'png' or 'tex'."
    exit 1
fi