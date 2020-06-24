file(GLOB source_files ${NEXUS_SOURCE_PATH}/BacktesterTests/*.cpp)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_executable(BacktesterTests ${source_files})
target_link_libraries(BacktesterTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(BacktesterTests
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
  add_custom_command(TARGET BacktesterTests PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET BacktesterTests PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
add_custom_command(TARGET BacktesterTests POST_BUILD COMMAND BacktesterTests)
install(TARGETS BacktesterTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS BacktesterTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
