#!/bin/sh

order=$1
shift

# Initialize
rm -rf dot/
rm -rf png/
rm -rf svg/

# Execute (forwards any extra flags, e.g. "improper", to the generator)
./build/feynman_diagram_generator $order "$@"

status=$?

if [ $status -eq 0 ]; then
    # SVG output is produced directly by the generator (in svg/); no external
    # tool needed. If Graphviz is available, also rasterize the dot files to PNG.
    if command -v dot >/dev/null 2>&1; then
        mkdir -p png
        for file in dot/*.dot; do
            # Check if the file exists to avoid errors if no .dot files are found
            [ -e "$file" ] || continue
            base_name=$(basename "$file" .dot)
            dot -Tpng "$file" -o "png/${base_name}.png"
            echo "Converted $file to png/${base_name}.png"
        done
    else
        echo "Graphviz 'dot' not found; skipping PNG conversion (vector SVG output is in svg/)."
    fi
fi
