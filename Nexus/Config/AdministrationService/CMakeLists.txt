file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/AdministrationServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/AdministrationServiceTests/*.cpp)
add_executable(AdministrationServiceTests ${header_files} ${source_files})
beam_configure_target(AdministrationServiceTests DEPENDENCIES CRYPTOPP SQLITE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(AdministrationServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET AdministrationServiceTests
  POST_BUILD COMMAND AdministrationServiceTests)
beam_install_target(
  AdministrationServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
