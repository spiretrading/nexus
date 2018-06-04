find_package(Qt5Widgets REQUIRED)

file(GLOB header_files ./../../../include/spire/toolbar/*.hpp)
file(GLOB source_files ./../../../source/toolbar/*.cpp)

add_library(toolbar ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(toolbar PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_toolbar)
install(TARGETS toolbar CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS toolbar CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
