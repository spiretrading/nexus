find_package(Qt5Widgets REQUIRED)
set(resource_files ${SPIRE_RESOURCES_PATH}/Resources.qrc)
qt5_add_resources(generated_resource_files ${resource_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Resources ${generated_resource_files})
beam_configure_target(Resources)
target_link_libraries(Resources BeamRoutines)
set_target_properties(Resources PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_resources)
beam_install_target(Resources "${SPIRE_LIBRARY_INSTALL_DIRECTORY}/$<CONFIG>")
