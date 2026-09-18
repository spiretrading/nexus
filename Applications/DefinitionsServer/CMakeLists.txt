cmake_minimum_required(VERSION 3.28)
project(DefinitionsServer LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
include_directories(${PROJECT_BINARY_DIR})
set(header_files "${PROJECT_BINARY_DIR}/Version.hpp")
file(GLOB_RECURSE source_files Source/*.cpp)
add_executable(DefinitionsServer ${header_files} ${source_files})
beam_configure_target(DefinitionsServer DEPENDENCIES
  BEAM BOOST CRYPTOPP OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
target_link_libraries(DefinitionsServer PUBLIC BeamRoutines)
target_compile_definitions(DefinitionsServer PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(WIN32)
  target_link_libraries(DefinitionsServer PUBLIC Crypt32.lib)
endif()
beam_install_target(DefinitionsServer "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/config.default.yml Application/countries.default.yml
  Application/country_code_parser.py Application/currencies.default.yml
  Application/date_time_zonespec.default.csv
  Application/destinations.default.yml Application/setup.py
  Application/trading_schedules.default.yml Application/venues.default.yml
  ../Python/setup_utils.py
  DESTINATION "${PROJECT_BINARY_DIR}/Application")
if(UNIX)
  install(PROGRAMS Application/check.sh Application/start.sh Application/stop.sh
    Application/update_countries.sh
    DESTINATION "${PROJECT_BINARY_DIR}/Application")
endif()
beam_configure_clean()
