file(GLOB header_files
  ./../../../Include/ChiaMarketDataFeedClient/*.hpp)
file(GLOB source_files
  ./../../../Source/*.cpp)

add_executable(ChiaMarketDataFeedClient ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(ChiaMarketDataFeedClient
  debug $ENV{YAML_LIBRARY_DEBUG_PATH}
  optimized $ENV{YAML_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized $ENV{OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized $ENV{OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{ZLIB_LIBRARY_DEBUG_PATH}
  optimized $ENV{ZLIB_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(ChiaMarketDataFeedClient
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
    dl pthread rt)
endif(UNIX)

install(TARGETS ChiaMarketDataFeedClient
  DESTINATION $ENV{INSTALL_DIRECTORY})
