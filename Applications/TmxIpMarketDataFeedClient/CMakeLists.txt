cmake_minimum_required(VERSION 3.28)
project(TmxIpMarketDataFeedClient LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
file(GLOB_RECURSE library_header_files Include/*.hpp)
add_library(Library INTERFACE)
target_sources(Library
  INTERFACE FILE_SET HEADERS BASE_DIRS Include FILES ${library_header_files})
target_include_directories(
  Library INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")
add_executable(Application Source/main.cpp "${PROJECT_BINARY_DIR}/Version.hpp")
set_target_properties(
  Application PROPERTIES OUTPUT_NAME TmxIpMarketDataFeedClient)
target_include_directories(Application PRIVATE "${PROJECT_BINARY_DIR}")
beam_configure_target(Application DEPENDENCIES
  BEAM BOOST CRYPTOPP OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
target_compile_definitions(Application PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(Application PRIVATE Library BeamRoutines)
if(WIN32)
  target_link_libraries(Application PRIVATE Crypt32.lib)
endif()
beam_install_target(Application "${PROJECT_BINARY_DIR}/Application")
file(GLOB test_source_files Source/Tests/*.cpp)
add_executable(Tests ${test_source_files})
set_target_properties(
  Tests PROPERTIES OUTPUT_NAME TmxIpMarketDataFeedClientTester)
beam_configure_target(Tests DEPENDENCIES
  BEAM BOOST CRYPTOPP DOCTEST OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
target_compile_definitions(Tests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(Tests PRIVATE Library BeamRoutines)
beam_install_target(Tests "${PROJECT_BINARY_DIR}/Tests")
add_custom_command(TARGET Tests POST_BUILD COMMAND Tests)
install(FILES Application/setup.py
  ../Python/setup_utils.py DESTINATION "${PROJECT_BINARY_DIR}/Application")
file(GLOB configuration_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/Application"
  Application/tmx_ip_*/config.default.yml)
foreach(configuration IN LISTS configuration_files)
  get_filename_component(directory "${configuration}" DIRECTORY)
  install(FILES "Application/${configuration}"
    DESTINATION "${PROJECT_BINARY_DIR}/Application/${directory}")
endforeach()
if(UNIX)
  install(PROGRAMS Application/check.sh Application/start.sh Application/stop.sh
    DESTINATION "${PROJECT_BINARY_DIR}/Application")
endif()
beam_configure_clean()
