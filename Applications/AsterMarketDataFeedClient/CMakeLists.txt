cmake_minimum_required(VERSION 3.28)
project(AsterMarketDataFeedClient LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(D "${CMAKE_BINARY_DIR}/Dependencies" CACHE STRING
  "Path to dependencies folder.")
file(TO_NATIVE_PATH "${D}" D)
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
if(WIN32)
  set(configure_script
    cmd /c "CALL ${CMAKE_SOURCE_DIR}/configure.bat -DD=${D}")
elseif(UNIX)
  set(configure_script "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    "${CMAKE_BUILD_TYPE}")
endif()
execute_process(COMMAND ${configure_script}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" RESULT_VARIABLE configure_result
  OUTPUT_VARIABLE configure_output ERROR_VARIABLE configure_error
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "Configuration script failed with error:\n${configure_error}\nOutput:\n${configure_output}")
endif()
include(../../Nexus/Config/dependencies.cmake)
include_directories(${NEXUS_INCLUDE_PATH})
include_directories(SYSTEM ${BEAM_INCLUDE_PATH})
include_directories(SYSTEM ${BOOST_INCLUDE_PATH})
include_directories(SYSTEM ${CRYPTOPP_INCLUDE_PATH})
include_directories(SYSTEM ${DOCTEST_INCLUDE_PATH})
include_directories(SYSTEM ${OPEN_SSL_INCLUDE_PATH})
include_directories(SYSTEM ${TCLAP_INCLUDE_PATH})
include_directories(SYSTEM ${YAML_INCLUDE_PATH})
include_directories(SYSTEM ${ZLIB_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
if(MSVC)
  add_compile_options(/bigobj /external:anglebrackets /external:W0
    $<$<CONFIG:Release>:/GL> /MP /WX /Zc:__cplusplus /Zc:preprocessor)
  add_compile_definitions(_CRT_SECURE_NO_DEPRECATE NOMINMAX
    _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
  add_link_options($<$<CONFIG:Release>:/LTCG>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-g $<$<CONFIG:Release>:-DNDEBUG>)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-fsized-deallocation)
  endif()
endif()
if(CYGWIN)
  add_compile_definitions(__USE_W32_SOCKETS)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  add_compile_options($<$<CONFIG:Release>:-pthreads>)
endif()
include_directories(Include)
include_directories(${PROJECT_BINARY_DIR})
file(GLOB_RECURSE library_header_files Include/AsterMarketDataFeedClient/*.hpp)
add_library(Library INTERFACE)
target_sources(Library
  PUBLIC FILE_SET HEADERS BASE_DIRS Include FILES ${library_header_files})
target_include_directories(Library INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/Include
  ${PROJECT_BINARY_DIR})
file(GLOB application_source_files Source/main.cpp)
add_executable(Application ${application_source_files} Version.hpp)
set_target_properties(
  Application PROPERTIES OUTPUT_NAME AsterMarketDataFeedClient)
target_link_libraries(Application Library
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(Application
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif()
if(WIN32)
  target_link_libraries(Application Crypt32.lib)
endif()
install(TARGETS Application DESTINATION ${PROJECT_BINARY_DIR}/Application)
file(GLOB test_source_files Source/Tests/*.cpp)
add_executable(Tests ${test_source_files})
set_target_properties(
  Tests PROPERTIES OUTPUT_NAME AsterMarketDataFeedClientTester)
target_link_libraries(Tests Library
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(Tests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif()
if(WIN32)
  target_link_libraries(Tests Crypt32.lib)
endif()
install(TARGETS Tests DESTINATION ${PROJECT_BINARY_DIR}/Tests)
add_custom_command(TARGET Tests POST_BUILD COMMAND Tests)
