# CMake for Iris Header Installation
# 2025 Copyright Ryan Landvater
# Created 3/3/2025
cmake_minimum_required(VERSION 3.11...3.30)

# Iris requires C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(IRIS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(IRIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/)
Project (IrisHeadersProj LANGUAGES CXX)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(IrisHeaders INTERFACE)
target_include_directories(
    IrisHeaders INTERFACE
    $<BUILD_INTERFACE:${IRIS_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_sources(
    IrisHeaders INTERFACE FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/
    FILES 
    ${IRIS_INCLUDE_DIR}/IrisTypes.hpp
    ${IRIS_INCLUDE_DIR}/IrisCore.hpp
    ${IRIS_INCLUDE_DIR}/IrisCodecTypes.hpp
    ${IRIS_INCLUDE_DIR}/IrisCodecCore.hpp
)
if (${PROJECT_IS_TOP_LEVEL})
    install(
        TARGETS IrisHeaders
        EXPORT IrisHeadersConfig
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Iris
    )
    install(
        EXPORT IrisHeadersConfig
        FILE IrisHeadersConfig.cmake
        NAMESPACE "Iris::"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Iris
    )
endif()

if (NOT ${PROJECT_IS_TOP_LEVEL})
    add_library(IrisBuffer STATIC)
    target_include_directories (
        IrisBuffer PUBLIC
        ${IRIS_INCLUDE_DIR}
    )
    target_sources ( 
        IrisBuffer PUBLIC ${IRIS_SOURCE_DIR}/IrisBuffer.cpp
    )
endif()