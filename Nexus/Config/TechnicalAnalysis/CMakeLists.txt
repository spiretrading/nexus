file(GLOB_RECURSE source_files
  ${NEXUS_SOURCE_PATH}/TechnicalAnalysisTests/*.cpp)
add_executable(TechnicalAnalysisTests ${source_files})
if(MSVC)
  target_precompile_headers(TechnicalAnalysisTests PRIVATE <doctest/doctest.h>
    <Beam/Queues/Queues.hpp> <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Nexus/Definitions/Definitions.hpp>
    <Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp>
    <Nexus/TechnicalAnalysis/TechnicalAnalysis.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/TechnicalAnalysisTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(TechnicalAnalysisTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
if(UNIX)
  target_link_libraries(TechnicalAnalysisTests
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
add_custom_command(TARGET TechnicalAnalysisTests
  POST_BUILD COMMAND TechnicalAnalysisTests)
install(TARGETS TechnicalAnalysisTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS TechnicalAnalysisTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
