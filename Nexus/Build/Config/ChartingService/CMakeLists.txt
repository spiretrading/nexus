file(GLOB header_files
  ./../../../Include/Nexus/ChartingService/*.hpp)
file(GLOB source_files
  ./../../../Source/ChartingService/*.cpp)

add_library(ChartingService ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(ChartingService PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_charting_service)
install(TARGETS ChartingService CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS ChartingService CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Nexus/ChartingServiceTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/ChartingServiceTests/*.cpp)

add_library(ChartingServiceTests ${test_header_files}
  ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(ChartingServiceTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_charting_service_tests)
install(TARGETS ChartingServiceTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS ChartingServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(ChartingServiceTester
  ./../../../Source/ChartingServiceTester/main.cpp)
target_link_libraries(ChartingServiceTester ChartingServiceTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(ChartingServiceTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
  target_link_libraries(ChartingServiceTester rt socket nsl)
endif()

add_custom_command(TARGET ChartingServiceTester POST_BUILD
  COMMAND ChartingServiceTester)
install(TARGETS ChartingServiceTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ChartingServiceTester CONFIGURATIONS Release
  RelWithDebInfo DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
