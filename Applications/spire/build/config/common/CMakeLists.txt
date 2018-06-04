find_package(Qt5Widgets REQUIRED)

file(GLOB header_files ./../../../include/spire/spire/*.hpp)
file(GLOB source_files ./../../../source/common/*.cpp)

add_library(common ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(common PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME common)
install(TARGETS common CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS common CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
