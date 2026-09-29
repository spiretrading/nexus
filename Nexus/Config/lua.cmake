cmake_minimum_required(VERSION 3.28)
project(Lua LANGUAGES C)

file(GLOB header_files *.h)
file(GLOB source_files *.c)
list(FILTER source_files EXCLUDE REGEX "/(lua|luac)\\.c$")
add_library(lua STATIC ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
