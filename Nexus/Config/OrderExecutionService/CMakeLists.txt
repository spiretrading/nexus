file(GLOB_RECURSE header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/OrderExecutionServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/OrderExecutionServiceTests/*.cpp)
add_executable(OrderExecutionServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(OrderExecutionServiceTests PRIVATE
    <doctest/doctest.h> <Beam/Collections/Collections.hpp> <Beam/IO/IO.hpp>
    <Beam/Pointers/Pointers.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Routines/Routines.hpp> <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocator/ServiceLocator.hpp>
    <Beam/ServiceLocatorTests/ServiceLocatorTests.hpp>
    <Beam/Services/Services.hpp> <Beam/ServicesTests/ServicesTests.hpp>
    <Beam/TimeService/TimeService.hpp>
    <Beam/UidServiceTests/UidServiceTests.hpp>
    <Nexus/Accounting/Accounting.hpp>
    <Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp>
    <Nexus/Definitions/Definitions.hpp>
    <Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp>
    <Nexus/OrderExecutionService/OrderExecutionService.hpp>
    <Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/OrderExecutionServiceTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(OrderExecutionServiceTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(OrderExecutionServiceTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(OrderExecutionServiceTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif()
add_custom_command(TARGET OrderExecutionServiceTests
  POST_BUILD COMMAND OrderExecutionServiceTests)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderExecutionServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
