file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/DefinitionsTests/*.cpp)
add_executable(DefinitionsTests ${source_files})
target_link_libraries(DefinitionsTests
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(DefinitionsTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
add_custom_command(TARGET DefinitionsTests POST_BUILD COMMAND DefinitionsTests)
install(TARGETS DefinitionsTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS DefinitionsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
