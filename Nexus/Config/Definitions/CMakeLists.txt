file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/DefinitionsTests/*.cpp)
add_executable(DefinitionsTests ${source_files})
if(MSVC)
  target_precompile_headers(DefinitionsTests PRIVATE <doctest/doctest.h>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/Utilities/Utilities.hpp> <Nexus/Definitions/Definitions.hpp>)
  set_source_files_properties(${NEXUS_SOURCE_PATH}/DefinitionsTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(DefinitionsTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_compile_definitions(DefinitionsTests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(DefinitionsTests
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(DefinitionsTests
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
add_custom_command(TARGET DefinitionsTests POST_BUILD COMMAND DefinitionsTests)
install(TARGETS DefinitionsTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS DefinitionsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
