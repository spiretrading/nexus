file(GLOB header_files
  ./../../../Include/Nexus/OrderExecutionService/*.hpp)
file(GLOB source_files
  ./../../../Source/OrderExecutionService/*.cpp)

add_library(OrderExecutionService ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(OrderExecutionService PROPERTIES
  OUTPUT_NAME nexus_order_execution_service)
install(TARGETS OrderExecutionService CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionService CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Nexus/OrderExecutionServiceTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/OrderExecutionServiceTests/*.cpp)

add_library(OrderExecutionServiceTests
  ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(OrderExecutionServiceTests PROPERTIES
  OUTPUT_NAME nexus_order_execution_service_tests)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(OrderExecutionServiceTester
  ./../../../Source/OrderExecutionServiceTester/main.cpp)
target_link_libraries(OrderExecutionServiceTester OrderExecutionServiceTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH})

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()
if(UNIX)
  target_link_libraries(OrderExecutionServiceTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_REGEX_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(OrderExecutionServiceTester rt socket nsl)
endif()

add_custom_command(TARGET OrderExecutionServiceTester POST_BUILD
  COMMAND OrderExecutionServiceTester)
install(TARGETS OrderExecutionServiceTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionServiceTester CONFIGURATIONS Release
  RelWithDebInfo DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
