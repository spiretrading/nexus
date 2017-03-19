file(GLOB header_files
  ./../../../Include/Nexus/DefinitionsService/*.hpp)
file(GLOB source_files
  ./../../../Source/DefinitionsService/*.cpp)

add_library(DefinitionsService ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(DefinitionsService PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_definitions_service)
install(TARGETS DefinitionsService CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS DefinitionsService CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
