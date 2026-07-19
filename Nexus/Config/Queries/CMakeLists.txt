file(GLOB_RECURSE header_files ${NEXUS_INCLUDE_PATH}/Nexus/QueriesTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/QueriesTests/*.cpp)
add_executable(QueriesTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(QueriesTests PRIVATE <doctest/doctest.h>
    <Beam/Queries/Queries.hpp>
    <Nexus/OrderExecutionService/OrderExecutionService.hpp>
    <Nexus/Queries/Queries.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/QueriesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(QueriesTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
if(UNIX)
  target_link_libraries(QueriesTests
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
add_custom_command(TARGET QueriesTests POST_BUILD COMMAND QueriesTests)
install(TARGETS QueriesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS QueriesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
