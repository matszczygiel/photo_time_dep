cmake_minimum_required(VERSION 3.8)

project (photo_td LANGUAGES CXX)

add_executable (main
                src/basis.cpp
                src/disk_reader.cpp
                src/control_data.cpp
                src/main.cpp)

target_compile_features(main PUBLIC
                        cxx_std_11)

target_include_directories(main PUBLIC src)
