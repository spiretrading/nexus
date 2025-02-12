cmake_minimum_required(VERSION 3.28)
project(Lua)

file(GLOB_RECURSE header_files ./*.h)
file(GLOB_RECURSE source_files ./*.c)

add_library(lua ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
