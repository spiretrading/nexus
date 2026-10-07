cmake_minimum_required(VERSION 3.28)
project(WebPortal LANGUAGES CXX)
include(../../Nexus/Config/project.cmake)
include(../../Nexus/Config/dependencies.cmake)
include("${beam_configuration}/targets.cmake")
include_directories(${NEXUS_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
include_directories(Include)
include_directories(${PROJECT_BINARY_DIR})
file(GLOB_RECURSE header_files Include/WebPortal/*.hpp)
list(APPEND header_files "${PROJECT_BINARY_DIR}/Version.hpp")
file(GLOB_RECURSE source_files Source/*.cpp)
list(REMOVE_ITEM source_files "${CMAKE_CURRENT_SOURCE_DIR}/Source/main.cpp")
set(web_portal_dependencies
  BEAM BOOST CRYPTOPP OPEN_SSL OPEN_SSL_BASE TCLAP YAML ZLIB)
add_library(WebPortalLibrary STATIC ${header_files} ${source_files})
beam_configure_target(WebPortalLibrary DEPENDENCIES ${web_portal_dependencies})
target_link_libraries(WebPortalLibrary PUBLIC BeamRoutines)
target_compile_definitions(WebPortalLibrary PUBLIC YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_property(GLOBAL APPEND PROPERTY BEAM_BUILD_TARGETS WebPortalLibrary)
add_executable(WebPortal Source/main.cpp)
beam_configure_target(WebPortal DEPENDENCIES ${web_portal_dependencies})
target_link_libraries(WebPortal PRIVATE WebPortalLibrary)
file(GLOB_RECURSE test_files Tests/*.cpp)
list(FILTER test_files EXCLUDE REGEX "/FunctionalTests/")
add_executable(WebPortalTests ${test_files})
beam_configure_target(WebPortalTests DEPENDENCIES
  ${web_portal_dependencies} DOCTEST)
target_link_libraries(WebPortalTests PRIVATE WebPortalLibrary)
add_custom_command(TARGET WebPortalTests POST_BUILD COMMAND WebPortalTests)
add_dependencies(WebPortal WebPortalTests)
if(WIN32)
  target_link_libraries(WebPortalLibrary PUBLIC Crypt32.lib ntdll.lib)
endif()
if(UNIX)
  find_library(BOOST_PROCESS_LIBRARY NAMES boost_process
    PATHS "${BOOST_OPTIMIZED_PATH}" NO_DEFAULT_PATH REQUIRED)
  find_library(BOOST_FILESYSTEM_LIBRARY NAMES boost_filesystem
    PATHS "${BOOST_OPTIMIZED_PATH}" NO_DEFAULT_PATH REQUIRED)
  target_link_libraries(WebPortalLibrary PUBLIC
    ${BOOST_PROCESS_LIBRARY} ${BOOST_FILESYSTEM_LIBRARY})
  add_executable(ReportProcessFixture EXCLUDE_FROM_ALL
    Tests/FunctionalTests/ReportProcessFixture.cpp)
  beam_configure_target(ReportProcessFixture DEPENDENCIES
    ${web_portal_dependencies})
  target_link_libraries(ReportProcessFixture PRIVATE WebPortalLibrary)
  add_executable(ReportProcessTests EXCLUDE_FROM_ALL
    Tests/FunctionalTests/ReportProcessTests.cpp Tests/main.cpp)
  beam_configure_target(ReportProcessTests DEPENDENCIES
    ${web_portal_dependencies} DOCTEST)
  target_link_libraries(ReportProcessTests PRIVATE WebPortalLibrary)
  target_compile_definitions(ReportProcessTests PRIVATE
    REPORT_PROCESS_FIXTURE="$<TARGET_FILE:ReportProcessFixture>")
  add_dependencies(ReportProcessTests ReportProcessFixture)
  add_custom_command(TARGET ReportProcessTests POST_BUILD
    COMMAND ReportProcessTests)
endif()
beam_install_target(WebPortal "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/config.default.yml Application/setup.py
  ../Python/setup_utils.py
  DESTINATION "${PROJECT_BINARY_DIR}/Application")
install(DIRECTORY
  DESTINATION "${PROJECT_BINARY_DIR}/Application/report_definitions")
if(UNIX)
  install(PROGRAMS Application/check.sh Application/start.sh Application/stop.sh
    DESTINATION "${PROJECT_BINARY_DIR}/Application")
endif()
beam_configure_clean()
