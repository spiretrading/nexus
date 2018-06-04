find_package(Qt5Widgets REQUIRED)

file(GLOB header_files ./../../../include/spire/login/*.hpp)
file(GLOB source_files ./../../../source/login/*.cpp)

add_library(login ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(login PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_login)
install(TARGETS login CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS login CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
