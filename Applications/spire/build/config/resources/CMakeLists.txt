find_package(Qt5Widgets REQUIRED)

set(resource_files ./../../../resources/resources.qrc)

qt5_add_resources(generated_resource_files ${resource_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(resources ${generated_resource_files})
set_target_properties(resources PROPERTIES OUTPUT_NAME spire_resources)
install(TARGETS resources CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS resources CONFIGURATIONS Release
DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
