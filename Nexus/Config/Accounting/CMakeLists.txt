file(
  GLOB_RECURSE header_files ${NEXUS_INCLUDE_PATH}/Nexus/AccountingTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/AccountingTests/*.cpp)
add_executable(AccountingTests ${header_files} ${source_files})
beam_configure_target(AccountingTests DEPENDENCIES CRYPTOPP)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(AccountingTests PUBLIC BeamRoutines)
add_custom_command(TARGET AccountingTests POST_BUILD COMMAND AccountingTests)
beam_install_target(AccountingTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
