cmake_minimum_required(VERSION 3.28)
project(TmxTl1MarketDataFeedClient LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
include_directories(Include)
include_directories(${PROJECT_BINARY_DIR})
file(GLOB_RECURSE header_files Include/TmxTl1MarketDataFeedClient/*.hpp)
list(APPEND header_files "${PROJECT_BINARY_DIR}/Version.hpp")
file(GLOB_RECURSE source_files Source/*.cpp)
add_executable(TmxTl1MarketDataFeedClient ${header_files} ${source_files})
beam_configure_target(TmxTl1MarketDataFeedClient DEPENDENCIES
  BEAM BOOST CRYPTOPP OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
target_link_libraries(TmxTl1MarketDataFeedClient PUBLIC BeamRoutines)
target_compile_definitions(
  TmxTl1MarketDataFeedClient PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(WIN32)
  target_link_libraries(TmxTl1MarketDataFeedClient PUBLIC Crypt32.lib)
endif()
beam_install_target(
  TmxTl1MarketDataFeedClient "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/setup.py
  ../Python/setup_utils.py DESTINATION "${PROJECT_BINARY_DIR}/Application")
file(GLOB configuration_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/Application"
  Application/tmxtl1_*/config.default.yml)
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
