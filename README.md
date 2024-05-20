# Feynman Diagram Generator

This project is a simple Feynman diagram generator written in C++ using the Boost Graph Library.

## Prerequisites

Ensure you have the following installed on your system:
- CMake (version 3.13 or higher)
- A C++ compiler (e.g., g++, clang)
- Boost library
- Graphviz (for generating graphical output)
- dot2tex (for generating `.tex` file for graph)

### Install dependencies on Debian-based systems:
```bash
sudo apt update
sudo apt install cmake g++ libboost-all-dev graphviz
pip install dot2tex
```

## Usage

1. Clone the repository:
```bash
git clone git@github.com:cohsh/feynman-diagram-generator.git
cd feynman-diagram-generator
```

2. Build the project:
```bash
make
```

3. Run the executable:
```bash
./build/feynman_diagram_generator tex
```
This will generate a `graph.pdf` file in the project directory.