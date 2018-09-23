file(GLOB header_files
  ./../../../Include/Nexus/OrderExecutionServiceTests/*.hpp)
file(GLOB source_files ./../../../Source/OrderExecutionServiceTests/*.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_executable(OrderExecutionServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(OrderExecutionServiceTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(OrderExecutionServiceTests
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(OrderExecutionServiceTests rt socket nsl)
endif()

add_custom_command(TARGET OrderExecutionServiceTests
  POST_BUILD COMMAND OrderExecutionServiceTests)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
