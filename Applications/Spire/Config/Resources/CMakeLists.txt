find_package(Qt5Widgets REQUIRED)

set(resource_files ${SPIRE_RESOURCES_PATH}/Resources.qrc)
qt5_add_resources(generated_resource_files ${resource_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Resources ${generated_resource_files})
set_target_properties(Resources PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_resources)
if(WIN32)
  add_custom_command(TARGET Resources PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET Resources PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
install(TARGETS Resources CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Resources CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
