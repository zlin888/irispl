cmake_minimum_required(VERSION 3.15)
project(iris)

set(CMAKE_CXX_STANDARD 20)

# CMAKE for boost
set(Boost_INCLUDE_DIR /usr/local/Cellar/boost/1.72.0/include)
set(Boost_LIBRARY_DIR /usr/local/Cellar/boost/1.72.0/lib)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(main main.cpp src/Parser.hpp src/Heap.hpp src/Analyser.hpp src/Compiler.hpp)
target_link_libraries( main ${Boost_LIBRARIES} )

add_executable(iris main.cpp)
