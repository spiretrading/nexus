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

file(GLOB test_header_files
  ./../../../Include/Nexus/DefinitionsServiceTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/DefinitionsServiceTests/*.cpp)

add_library(DefinitionsServiceTests ${test_header_files}
  ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(DefinitionsServiceTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_definitions_service_tests)
install(TARGETS DefinitionsServiceTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS DefinitionsServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
