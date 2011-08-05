cmake_minimum_required(VERSION 2.8)
    
project(KeVaDB)

add_executable(kevadb main.cpp io/driver.cpp io/file.cpp database.cpp)

target_link_libraries(kevadb rt)

add_subdirectory(io)
