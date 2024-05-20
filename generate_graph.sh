#!/bin/sh

generate_diagram() {
    ./build/feynman_diagram_generator
}

output_type=$1

if [ "$output_type" = "png" ]; then
    generate_diagram
    # Convert all .dot files in the "dot" directory to .png
    mkdir -p png
    for file in dot/*.dot; do
        # Check if the file exists to avoid errors if no .dot files are found
        base_name=$(basename "$file" .dot)
        dot -Tpng "$file" -o "png/${base_name}.png"
        echo "Converted $file to dot/${base_name}.png"
    done

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