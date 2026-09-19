cmake_minimum_required(VERSION 3.28)
project(AsxTradeItchMarketDataFeedClient LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
include_directories(${PROJECT_BINARY_DIR})
add_executable(Application Source/main.cpp "${PROJECT_BINARY_DIR}/Version.hpp")
set_target_properties(
  Application PROPERTIES OUTPUT_NAME AsxTradeItchMarketDataFeedClient)
beam_configure_target(Application DEPENDENCIES
  BEAM BOOST CRYPTOPP OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
target_link_libraries(Application PUBLIC BeamRoutines)
target_compile_definitions(Application PRIVATE YAML_CPP_STATIC_DEFINE)
if(WIN32)
  target_link_libraries(Application PUBLIC Crypt32.lib)
endif()
beam_install_target(Application "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/setup.py
  ../Python/setup_utils.py DESTINATION "${PROJECT_BINARY_DIR}/Application")
file(GLOB configuration_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/Application"
  Application/partition*/config.default.yml)
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
