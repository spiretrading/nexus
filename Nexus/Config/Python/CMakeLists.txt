file(GLOB_RECURSE header_files ${NEXUS_INCLUDE_PATH}/Nexus/Python/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/Python/*.cpp)
include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
add_library(Python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Python PROPERTIES OUTPUT_NAME nexus)
target_compile_definitions(Python
  PRIVATE ASPEN_USE_DLL BEAM_USE_DLL NEXUS_BUILD_DLL BEAM_ENABLE_STACK_PRINT)
set_target_properties(Python PROPERTIES CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN 0)
if(MSVC)
  target_compile_definitions(Python PRIVATE HAVE_SNPRINTF)
  set_target_properties(Python PROPERTIES SUFFIX ".pyd")
endif()
target_compile_options(Python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fvisibility=hidden>)
if(UNIX)
  set_target_properties(Python PROPERTIES PREFIX "" INSTALL_RPATH "$ORIGIN")
endif()
target_link_libraries(Python
  debug ${ASPEN_PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${ASPEN_PYTHON_LIBRARY_OPTIMIZED_PATH}
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${MYSQL_LIBRARY_DEBUG_PATH}
  optimized ${MYSQL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${BEAM_PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${BEAM_PYTHON_LIBRARY_OPTIMIZED_PATH}
  debug ${PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${PYTHON_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(Python
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    dl)
endif()
if(WIN32)
  target_link_libraries(Python bcrypt crypt32 Secur32 shlwapi)
endif()
install(TARGETS Python CONFIGURATIONS Debug
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Python CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Release)
