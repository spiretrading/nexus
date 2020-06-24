file(GLOB header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/OrderExecutionServiceTests/*.hpp)
file(GLOB source_files ${NEXUS_SOURCE_PATH}/OrderExecutionServiceTests/*.cpp)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_executable(OrderExecutionServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(OrderExecutionServiceTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(OrderExecutionServiceTests
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
  add_custom_command(TARGET OrderExecutionServiceTests PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET OrderExecutionServiceTests PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
add_custom_command(TARGET OrderExecutionServiceTests
  POST_BUILD COMMAND OrderExecutionServiceTests)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
