file(GLOB header_files
  ./../../../Include/CppTemplate/*.hpp)
file(GLOB source_files
  ./../../../Source/*.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

add_executable(CppTemplate ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(CppTemplate)

if(UNIX)
  target_link_libraries(CppTemplate)
endif(UNIX)

install(TARGETS CppTemplate DESTINATION $ENV{INSTALL_DIRECTORY})
