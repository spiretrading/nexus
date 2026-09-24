set(D "${CMAKE_BINARY_DIR}/Dependencies" CACHE STRING
  "Path to dependencies folder.")
get_filename_component(beam_configuration "${D}/Beam/Beam/Config" ABSOLUTE
  BASE_DIR "${PROJECT_BINARY_DIR}")
if(NOT EXISTS "${beam_configuration}/project.cmake")
  if(WIN32)
    set(configure_script
      cmd /c CALL "${PROJECT_SOURCE_DIR}/configure.bat" -DD "${D}"
      "${CMAKE_BUILD_TYPE}")
  elseif(UNIX)
    set(configure_script "${PROJECT_SOURCE_DIR}/configure.sh" "-DD=${D}"
      "${CMAKE_BUILD_TYPE}")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env NEXUS_SKIP_CMAKE=1
    ${configure_script}
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}" RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output ERROR_VARIABLE configure_error
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Dependency bootstrap failed with error:\n"
      "${configure_error}\nOutput:\n${configure_output}")
  endif()
endif()
include("${beam_configuration}/project.cmake")
beam_configure_project("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
  ENVIRONMENT NEXUS_SKIP_CMAKE=1)
