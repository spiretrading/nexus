file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/FixUtilitiesTests/*.cpp)
add_executable(FixUtilitiesTests ${source_files})
beam_configure_target(FixUtilitiesTests DEPENDENCIES QUICK_FIX)
if(WIN32)
  target_link_libraries(
    FixUtilitiesTests PUBLIC crypt32 Secur32 shlwapi ws2_32.lib)
endif()
target_link_libraries(FixUtilitiesTests PUBLIC BeamRoutines)
add_custom_command(TARGET FixUtilitiesTests
  POST_BUILD COMMAND FixUtilitiesTests)
beam_install_target(FixUtilitiesTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
