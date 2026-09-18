file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/RiskServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/RiskServiceTests/*.cpp)
add_executable(RiskServiceTests ${header_files} ${source_files})
beam_configure_target(RiskServiceTests DEPENDENCIES CRYPTOPP SQLITE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(RiskServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET RiskServiceTests POST_BUILD COMMAND RiskServiceTests)
beam_install_target(RiskServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
