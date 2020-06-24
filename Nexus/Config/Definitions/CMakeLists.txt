file(GLOB source_files ${NEXUS_SOURCE_PATH}/DefinitionsTests/*.cpp)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_executable(DefinitionsTests ${source_files})
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
if(WIN32)
  add_custom_command(TARGET DefinitionsTests PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET DefinitionsTests PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
add_custom_command(TARGET DefinitionsTests POST_BUILD COMMAND DefinitionsTests)
install(TARGETS DefinitionsTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS DefinitionsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
