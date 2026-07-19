file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/ChartingServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/ChartingServiceTests/*.cpp)
add_executable(ChartingServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(ChartingServiceTests PRIVATE <doctest/doctest.h>
    <Beam/IO/IO.hpp> <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocator/ServiceLocator.hpp> <Beam/Services/Services.hpp>
    <Beam/ServicesTests/ServicesTests.hpp> <Beam/TimeService/TimeService.hpp>
    <Nexus/ChartingService/ChartingService.hpp>
    <Nexus/Definitions/Definitions.hpp>
    <Nexus/MarketDataService/MarketDataService.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/ChartingServiceTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(ChartingServiceTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(ChartingServiceTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(ChartingServiceTests
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
add_custom_command(TARGET ChartingServiceTests
  POST_BUILD COMMAND ChartingServiceTests)
install(TARGETS ChartingServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ChartingServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
