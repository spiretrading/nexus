file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/ChartingServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/ChartingServiceTests/*.cpp)
add_executable(ChartingServiceTests ${header_files} ${source_files})
beam_configure_target(ChartingServiceTests DEPENDENCIES CRYPTOPP)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(ChartingServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET ChartingServiceTests
  POST_BUILD COMMAND ChartingServiceTests)
beam_install_target(ChartingServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
