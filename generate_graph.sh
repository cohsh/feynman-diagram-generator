#!/bin/sh

order=$1

# Initialize
rm -rf dot/
rm -rf png/

# Make output directory
mkdir -p png


# Execute
./build/feynman_diagram_generator $order

status=$?

if [ $status -eq 0 ]; then
    # Convert all .dot files in the "dot" directory to .png
    for file in dot/*.dot; do
        # Check if the file exists to avoid errors if no .dot files are found
        base_name=$(basename "$file" .dot)
        dot -Tpng "$file" -o "png/${base_name}.png"
        echo "Converted $file to dot/${base_name}.png"
    done
fi