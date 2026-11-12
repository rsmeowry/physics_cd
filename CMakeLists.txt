cmake_minimum_required(VERSION 4.0)
project(physics_cd)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(raylib)

add_executable(physics_cd main.cpp)
target_link_libraries(physics_cd raylib)