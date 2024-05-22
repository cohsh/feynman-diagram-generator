# Specify version
cmake_minimum_required(VERSION 3.13)

# Set the name of project and language
project(feynman_diagram_generator LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost
find_package(Boost REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIRS} include)

# Add the source files
set(SOURCES
    src/main.cpp
    src/graph.cpp
    src/bfs_dfs.cpp
    src/utility.cpp
)

# Add the executable
add_executable(feynman_diagram_generator ${SOURCES})

# Link Boost libraries
target_link_libraries(feynman_diagram_generator Boost::boost)