file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/SoupBinTcpTests/*.cpp)
add_executable(SoupBinTcpTests ${source_files})
if(MSVC)
  target_precompile_headers(SoupBinTcpTests PRIVATE <doctest/doctest.h>
    <Beam/IO/IO.hpp> <Beam/TimeService/TimeService.hpp>
    <Nexus/SoupBinTcp/SoupBinTcp.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/SoupBinTcpTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(SoupBinTcpTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(SoupBinTcpTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(SoupBinTcpTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
add_custom_command(TARGET SoupBinTcpTests POST_BUILD COMMAND SoupBinTcpTests)
install(TARGETS SoupBinTcpTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS SoupBinTcpTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
