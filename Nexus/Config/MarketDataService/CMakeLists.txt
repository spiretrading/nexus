file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/MarketDataServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/MarketDataServiceTests/*.cpp)
add_executable(MarketDataServiceTests ${header_files} ${source_files})
beam_configure_target(MarketDataServiceTests DEPENDENCIES CRYPTOPP SQLITE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(MarketDataServiceTests PUBLIC BeamRoutines)
add_custom_command(TARGET MarketDataServiceTests
  POST_BUILD COMMAND MarketDataServiceTests)
beam_install_target(
  MarketDataServiceTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
