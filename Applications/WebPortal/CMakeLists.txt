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
if(UNIX)
  list(APPEND web_portal_dependencies BOOST_PROCESS BOOST_FILESYSTEM)
endif()
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
file(GLOB_RECURSE functional_test_header_files
  Include/WebPortal/Tests/FunctionalTests/*.hpp Tests/FunctionalTests/*.hpp)
file(GLOB_RECURSE functional_test_source_files Tests/FunctionalTests/*.cpp)
list(REMOVE_ITEM functional_test_source_files
  "${CMAKE_CURRENT_SOURCE_DIR}/Tests/FunctionalTests/ReportProcessFixture.cpp")
if(NOT UNIX)
  list(REMOVE_ITEM functional_test_source_files
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests/FunctionalTests/ReportProcessTests.cpp")
endif()
add_executable(WebPortalFunctionalTests EXCLUDE_FROM_ALL
  ${functional_test_header_files} ${functional_test_source_files}
  Tests/main.cpp)
beam_configure_target(WebPortalFunctionalTests DEPENDENCIES
  ${web_portal_dependencies} DOCTEST)
target_link_libraries(WebPortalFunctionalTests PRIVATE WebPortalLibrary)
add_custom_command(TARGET WebPortalFunctionalTests POST_BUILD
  COMMAND WebPortalFunctionalTests)
if(WIN32)
  target_link_libraries(WebPortalLibrary PUBLIC Crypt32.lib ntdll.lib)
endif()
if(UNIX)
  add_executable(ReportProcessFixture EXCLUDE_FROM_ALL
    Tests/FunctionalTests/ReportProcessFixture.cpp)
  beam_configure_target(ReportProcessFixture DEPENDENCIES
    ${web_portal_dependencies})
  target_link_libraries(ReportProcessFixture PRIVATE WebPortalLibrary)
  target_compile_definitions(WebPortalFunctionalTests PRIVATE
    REPORT_PROCESS_FIXTURE="$<TARGET_FILE:ReportProcessFixture>")
  add_dependencies(WebPortalFunctionalTests ReportProcessFixture)
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
