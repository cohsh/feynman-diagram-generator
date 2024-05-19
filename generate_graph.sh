#!/bin/sh

./build/feynman_diagram_generator

dot -Tpng graph.dot -o graph.png