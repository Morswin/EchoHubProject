cmake_minimum_required(VERSION 4.3)
project(EchoHubProject)

add_subdirectory(external/SDL)

add_executable(echo_hub src/main.cpp)

target_link_libraries(echo_hub PRIVATE SDL3::SDL3-shared)
