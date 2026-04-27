cmake_minimum_required(VERSION 3.20)
project(GraphVisualizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(GraphVisualizer WIN32 ${SOURCES})

target_link_libraries(GraphVisualizer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_include_directories(GraphVisualizer PRIVATE src)
