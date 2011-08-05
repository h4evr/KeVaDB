cmake_minimum_required(VERSION 2.8)
    
project(database)

add_executable(database main.cpp io/driver.cpp io/file.cpp database.cpp)

target_link_libraries(database rt)

add_subdirectory(io)
