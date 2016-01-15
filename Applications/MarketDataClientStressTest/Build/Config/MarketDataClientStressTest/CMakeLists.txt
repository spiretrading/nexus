file(GLOB header_files
  ./../../../Include/MarketDataClientStressTest/*.hpp)
file(GLOB source_files
  ./../../../Source/*.cpp)

add_executable(MarketDataClientStressTest ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(MarketDataClientStressTest
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{YAML_LIBRARY_DEBUG_PATH}
  optimized $ENV{YAML_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{ZLIB_LIBRARY_DEBUG_PATH}
  optimized $ENV{ZLIB_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(MarketDataClientStressTest
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
    pthread rt)
endif(UNIX)
if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

install(TARGETS MarketDataClientStressTest DESTINATION $ENV{INSTALL_DIRECTORY})
