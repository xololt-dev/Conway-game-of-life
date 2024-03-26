cmake_minimum_required(VERSION 3.0.0)
project(Conway VERSION 0.1.0 LANGUAGES C CXX)

set(SOURCE_DIR   src)
set(INCLUDE_DIR  include)

include(CTest)
enable_testing()

###########
# Project #
###########
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# add_executable(${PROJECT_NAME} main.cpp)
include_directories(include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
