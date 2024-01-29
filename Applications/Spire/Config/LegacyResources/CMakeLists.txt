find_package(Qt5Widgets REQUIRED)

set(legacy_resource_files ${SPIRE_LEGACY_RESOURCES_PATH}/Resources.qrc)
qt5_add_resources(generated_legacy_resource_files ${legacy_resource_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(LegacyResources ${generated_legacy_resource_files})
set_target_properties(LegacyResources PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_legacy_resources)
install(TARGETS LegacyResources CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS LegacyResources CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
