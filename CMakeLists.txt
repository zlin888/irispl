cmake_minimum_required(VERSION 3.15)
project(typed_scheme)

set(CMAKE_CXX_STANDARD 20)

# CMAKE for boost
set(Boost_INCLUDE_DIR /usr/local/Cellar/boost/1.72.0/include)
set(Boost_LIBRARY_DIR /usr/local/Cellar/boost/1.72.0/lib)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(main main.cpp lib/Parser.hpp lib/Heap.hpp lib/Analyser.hpp lib/Compiler.hpp)
target_link_libraries( main ${Boost_LIBRARIES} )

add_executable(typed_scheme main.cpp)

add_subdirectory(Catch_tests)
