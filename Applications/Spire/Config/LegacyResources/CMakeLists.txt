find_package(Qt5Widgets REQUIRED)
set(legacy_resource_files ${SPIRE_LEGACY_RESOURCES_PATH}/LegacyResources.qrc)
qt5_add_resources(generated_legacy_resource_files ${legacy_resource_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(LegacyResources ${generated_legacy_resource_files})
beam_configure_target(LegacyResources)
target_link_libraries(LegacyResources BeamRoutines)
set_target_properties(LegacyResources PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_legacy_resources)
beam_install_target(
  LegacyResources "${SPIRE_LIBRARY_INSTALL_DIRECTORY}/$<CONFIG>")
