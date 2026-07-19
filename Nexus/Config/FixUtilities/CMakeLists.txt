file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/FixUtilitiesTests/*.cpp)
add_executable(FixUtilitiesTests ${source_files})
if(MSVC)
  target_precompile_headers(FixUtilitiesTests PRIVATE <doctest/doctest.h>
    <Beam/Queues/Queues.hpp> <Nexus/Definitions/Definitions.hpp>
    <Nexus/FixUtilities/FixUtilities.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/FixUtilitiesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(FixUtilitiesTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(FixUtilitiesTests
  debug ${QUICK_FIX_LIBRARY_DEBUG_PATH}
  optimized ${QUICK_FIX_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(FixUtilitiesTests
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
if(WIN32)
  target_link_libraries(FixUtilitiesTests crypt32 Secur32 shlwapi ws2_32.lib)
endif()
add_custom_command(TARGET FixUtilitiesTests
  POST_BUILD COMMAND FixUtilitiesTests)
install(TARGETS FixUtilitiesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS FixUtilitiesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
