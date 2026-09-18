file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/SimulationMatcherTests/*.cpp)
add_executable(SimulationMatcherTests ${source_files})
beam_configure_target(SimulationMatcherTests DEPENDENCIES CRYPTOPP SQLITE)
target_link_libraries(SimulationMatcherTests PUBLIC BeamRoutines)
add_custom_command(
  TARGET SimulationMatcherTests POST_BUILD COMMAND SimulationMatcherTests)
beam_install_target(
  SimulationMatcherTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
