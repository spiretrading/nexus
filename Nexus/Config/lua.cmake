cmake_minimum_required(VERSION 2.8)
project(Lua)

file(GLOB header_files ./*.h)
file(GLOB source_files ./*.c)

add_library(lua ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
