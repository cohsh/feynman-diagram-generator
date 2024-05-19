# Specify version
cmake_minimum_required(VERSION 3.13)

# Set the name of project and language
project(feynman_diagram_generator LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Make executable from src/main.cpp
add_executable(feynman_diagram_generator src/main.cpp)