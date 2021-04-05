cmake_minimum_required(VERSION 3.16)
project(smart-tv)
set(CMAKE_CXX_STANDARD 14)

find_package(nlohmann_json 3.7.0 REQUIRED)
find_package(PkgConfig)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json PkgConfig::Pistache)