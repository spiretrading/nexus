file(GLOB_RECURSE header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/DefinitionsServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/DefinitionsServiceTests/*.cpp)
add_executable(DefinitionsServiceTests ${header_files} ${source_files})
beam_configure_target(DefinitionsServiceTests DEPENDENCIES CRYPTOPP)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(DefinitionsServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET DefinitionsServiceTests
  POST_BUILD COMMAND DefinitionsServiceTests)
beam_install_target(
  DefinitionsServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
