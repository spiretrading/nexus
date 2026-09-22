cmake_minimum_required(VERSION 3.28)
project(OasisOrderExecutionServer LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
include_directories(Include)
include_directories(${PROJECT_BINARY_DIR})
file(GLOB_RECURSE header_files Include/OasisOrderExecutionServer/*.hpp)
list(APPEND header_files "${PROJECT_BINARY_DIR}/Version.hpp")
file(GLOB_RECURSE source_files Source/*.cpp)
add_executable(OasisOrderExecutionServer ${header_files} ${source_files})
beam_configure_target(OasisOrderExecutionServer DEPENDENCIES
  BEAM BOOST CRYPTOPP MYSQL OPEN_SSL OPEN_SSL_BASE QUICK_FIX TCLAP VIPER YAML
  ZLIB)
target_link_libraries(OasisOrderExecutionServer PUBLIC BeamRoutines)
target_compile_definitions(
  OasisOrderExecutionServer PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(WIN32)
  target_link_libraries(
    OasisOrderExecutionServer PUBLIC Crypt32 Secur32 shlwapi)
endif()
beam_install_target(
  OasisOrderExecutionServer "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/config.default.yml Application/fee_table.default.yml
  Application/asx.default.cfg Application/serenity.default.cfg
  Application/pure_designated_parser.py Application/tsx_etfs_parser.py
  Application/tsx_interlisted_parser.py Application/setup.py
  ../Python/setup_utils.py
  DESTINATION "${PROJECT_BINARY_DIR}/Application")
if(UNIX)
  install(PROGRAMS Application/check.sh Application/start.sh Application/stop.sh
    DESTINATION "${PROJECT_BINARY_DIR}/Application")
endif()
beam_configure_clean()
