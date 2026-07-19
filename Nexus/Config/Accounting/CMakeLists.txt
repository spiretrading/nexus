file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/AccountingTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/AccountingTests/*.cpp)
add_executable(AccountingTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(AccountingTests PRIVATE <doctest/doctest.h>
    <Beam/Queues/Queues.hpp> <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocatorTests/ServiceLocatorTests.hpp>
    <Beam/UidServiceTests/UidServiceTests.hpp> <Beam/Utilities/Utilities.hpp>
    <Nexus/Accounting/Accounting.hpp>
    <Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp>
    <Nexus/Definitions/Definitions.hpp>
    <Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp>
    <Nexus/OrderExecutionService/OrderExecutionService.hpp>
    <Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/AccountingTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(AccountingTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(AccountingTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(AccountingTests
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
add_custom_command(TARGET AccountingTests POST_BUILD COMMAND AccountingTests)
install(TARGETS AccountingTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS AccountingTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
