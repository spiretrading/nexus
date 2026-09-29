file(GLOB_RECURSE header_files ${NEXUS_INCLUDE_PATH}/Nexus/Python/*.hpp)
file(GLOB_RECURSE source_files ${NEXUS_SOURCE_PATH}/Python/*.cpp)
include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
add_library(Python SHARED ${header_files} ${source_files})
beam_configure_target(Python DEPENDENCIES ASPEN_PYTHON BEAM_PYTHON CRYPTOPP
  MYSQL OPEN_SSL OPEN_SSL_BASE PYTHON SQLITE YAML ZLIB)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Python PROPERTIES OUTPUT_NAME nexus)
target_compile_definitions(Python
  PRIVATE ASPEN_USE_DLL BEAM_ENABLE_STACK_PRINT BEAM_USE_DLL NEXUS_BUILD_DLL
  YAML_CPP_STATIC_DEFINE)
set_target_properties(Python PROPERTIES CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN 0)
if(MSVC)
  target_compile_definitions(Python PRIVATE HAVE_SNPRINTF)
  set_target_properties(Python PROPERTIES SUFFIX ".pyd")
endif()
target_compile_options(Python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fvisibility=hidden>)
if(UNIX)
  set_target_properties(Python PROPERTIES PREFIX "")
  if(APPLE)
    set_target_properties(Python PROPERTIES
      SUFFIX ".so" INSTALL_RPATH "@loader_path")
    target_link_options(Python PRIVATE "LINKER:-undefined,dynamic_lookup")
  else()
    set_target_properties(Python PROPERTIES INSTALL_RPATH "$ORIGIN")
  endif()
endif()
target_link_libraries(Python PUBLIC BeamRoutines)
if(UNIX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Python PUBLIC stdc++exp)
endif()
if(WIN32)
  target_link_libraries(Python PUBLIC bcrypt crypt32 Secur32 shlwapi Ws2_32)
endif()
beam_install_target(Python "${LIB_INSTALL_DIRECTORY}/$<CONFIG>")
set(python_modules "$<TARGET_FILE:Python>")
foreach(dependency ASPEN BEAM)
  string(CONCAT library
    "$<$<CONFIG:Debug>:${${dependency}_PYTHON_LIBRARY_DEBUG_PATH}>"
    "$<$<NOT:$<CONFIG:Debug>>:${${dependency}_PYTHON_LIBRARY_OPTIMIZED_PATH}>")
  if(WIN32)
    set(library "$<PATH:REPLACE_EXTENSION,LAST_ONLY,${library},.pyd>")
  endif()
  list(APPEND python_modules "${library}")
  install(FILES "${library}" DESTINATION "${LIB_INSTALL_DIRECTORY}/$<CONFIG>")
  set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
    "${LIB_INSTALL_DIRECTORY}/$<CONFIG>/$<PATH:GET_FILENAME,${library}>")
endforeach()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(TARGET Python POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory
    "${LIB_INSTALL_DIRECTORY}/$<CONFIG>"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${python_modules}
    "${LIB_INSTALL_DIRECTORY}/$<CONFIG>"
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tester.py"
    --config "$<CONFIG>"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  VERBATIM)
