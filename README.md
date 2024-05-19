# Feynman Diagram Generator

This project is a simple Feynman diagram generator written in C++ using the Boost Graph Library.

## Prerequisites

Ensure you have the following installed on your system:
- CMake (version 3.13 or higher)
- A C++ compiler (e.g., g++, clang)
- Boost library
- Graphviz (for generating graphical output)

### Install dependencies on Debian-based systems:
```bash
sudo apt update
sudo apt install cmake g++ libboost-all-dev graphviz
```

## Usage

1. Clone the repository:
```bash
git clone git@github.com:cohsh/feynman-diagram-generator.git
cd feynman-diagram-generator
```

2. Create a build directory and run CMake:
```bash
cmake -S . -B build
```

3. Build the project:
```bash
cmake --build build
```

4. Run the executable:
```bash
./build/feynman_diagram_generator
```
This will generate a `graph.dot` file in the project directory.

5. Generate the graphical output:
Use Graphviz to convert the `graph.dot` file to a PNG image:
```bash
dot -Tpng graph.dot -o graph.png
```