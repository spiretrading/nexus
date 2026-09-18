file(GLOB_RECURSE header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/OrderExecutionServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/OrderExecutionServiceTests/*.cpp)
add_executable(OrderExecutionServiceTests ${header_files} ${source_files})
beam_configure_target(OrderExecutionServiceTests DEPENDENCIES CRYPTOPP SQLITE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(OrderExecutionServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET OrderExecutionServiceTests
  POST_BUILD COMMAND OrderExecutionServiceTests)
beam_install_target(
  OrderExecutionServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
