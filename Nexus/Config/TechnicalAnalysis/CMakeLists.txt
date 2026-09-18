file(
  GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/TechnicalAnalysisTests/*.cpp)
add_executable(TechnicalAnalysisTests ${source_files})
beam_configure_target(TechnicalAnalysisTests)
target_link_libraries(TechnicalAnalysisTests PUBLIC BeamRoutines)
add_custom_command(TARGET TechnicalAnalysisTests
  POST_BUILD COMMAND TechnicalAnalysisTests)
beam_install_target(
  TechnicalAnalysisTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
