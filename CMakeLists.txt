cmake_minimum_required(VERSION 3.8)

project (photo_td LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    message("debug mode")
endif()

add_executable (main
                src/basis.cpp
                src/disk_reader.cpp
                src/control_data.cpp
                src/basis.h
                src/disk_reader.h
                src/control_data.h
                src/main.cpp
                src/utils.h)

target_compile_features(main PUBLIC
                        cxx_std_11)

target_compile_options(main PRIVATE -Wall )

target_include_directories(main PUBLIC src)

target_compile_definitions(main PUBLIC "$<$<CONFIG:DEBUG>:PHOTO_DEBUG>")

find_package(OpenMP REQUIRED)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

target_link_libraries (main PRIVATE OpenMP::OpenMP_CXX Eigen3::Eigen)
