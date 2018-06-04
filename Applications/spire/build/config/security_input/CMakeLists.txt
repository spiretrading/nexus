find_package(Qt5Widgets REQUIRED)

file(GLOB header_files ./../../../include/spire/security_input/*.hpp)
file(GLOB source_files ./../../../source/security_input/*.cpp)

add_library(security_input ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(security_input PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_security_input)
install(TARGETS security_input CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS security_input CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
