file(GLOB header_files
  ./../../../Include/Nexus/MarketDataService/*.hpp)
file(GLOB source_files
  ./../../../Source/MarketDataService/*.cpp)

add_library(MarketDataService ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(MarketDataService PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
  OUTPUT_NAME nexus_market_data_service)
install(TARGETS MarketDataService CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS MarketDataService CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Nexus/MarketDataServiceTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/MarketDataServiceTests/*.cpp)

add_library(MarketDataServiceTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(MarketDataServiceTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
  OUTPUT_NAME nexus_market_data_service_tests)
install(TARGETS MarketDataServiceTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS MarketDataServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(MarketDataServiceTester
  ./../../../Source/MarketDataServiceTester/main.cpp)
target_link_libraries(MarketDataServiceTester MarketDataServiceTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(MarketDataServiceTester
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
if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
  target_link_libraries(MarketDataServiceTester rt socket nsl)
endif()

add_custom_command(TARGET MarketDataServiceTester POST_BUILD
  COMMAND MarketDataServiceTester)
install(TARGETS MarketDataServiceTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS MarketDataServiceTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
