file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/AdministrationServiceTests/*.hpp)
file(GLOB_RECURSE
  source_files ${NEXUS_SOURCE_PATH}/AdministrationServiceTests/*.cpp)
add_executable(AdministrationServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(AdministrationServiceTests PRIVATE
    <doctest/doctest.h> <Beam/IO/IO.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocator/ServiceLocator.hpp>
    <Beam/ServiceLocatorTests/ServiceLocatorTests.hpp>
    <Beam/Services/Services.hpp> <Beam/ServicesTests/ServicesTests.hpp>
    <Beam/TimeService/TimeService.hpp>
    <Nexus/AdministrationService/AdministrationService.hpp>
    <Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp>
    <Nexus/MarketDataService/MarketDataService.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/AdministrationServiceTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(AdministrationServiceTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(AdministrationServiceTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(AdministrationServiceTests
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
add_custom_command(TARGET AdministrationServiceTests
  POST_BUILD COMMAND AdministrationServiceTests)
install(TARGETS AdministrationServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS AdministrationServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
