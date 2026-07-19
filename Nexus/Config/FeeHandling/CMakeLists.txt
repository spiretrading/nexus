file(GLOB_RECURSE
  header_files ${NEXUS_INCLUDE_PATH}/Nexus/FeeHandlingTests/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/FeeHandlingTests/*.cpp)
add_executable(FeeHandlingTests ${header_files} ${source_files})
target_compile_definitions(FeeHandlingTests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(FeeHandlingTests
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(FeeHandlingTests PRIVATE <doctest/doctest.h>
    <Beam/Pointers/Pointers.hpp> <Beam/Utilities/Utilities.hpp>
    <Nexus/Definitions/Definitions.hpp> <Nexus/FeeHandling/FeeHandling.hpp>
    <Nexus/FeeHandlingTests/FeeHandlingTests.hpp>
    <Nexus/OrderExecutionService/OrderExecutionService.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/FeeHandlingTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(FeeHandlingTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
if(UNIX)
  target_link_libraries(FeeHandlingTests
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
add_custom_command(TARGET FeeHandlingTests POST_BUILD COMMAND FeeHandlingTests)
install(TARGETS FeeHandlingTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS FeeHandlingTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
