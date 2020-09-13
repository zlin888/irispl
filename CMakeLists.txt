cmake_minimum_required(VERSION 3.15)
project(iris)

set(CMAKE_CXX_STANDARD 20)
set(EXEC iris)

FIND_PACKAGE(Boost REQUIRED)

add_executable(${EXEC} main.cpp)
TARGET_LINK_LIBRARIES(${EXEC}
	${BOOST_LIBRARIES}
)
