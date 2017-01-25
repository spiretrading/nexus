file(GLOB header_files
  ./../../../Include/Nexus/ServiceClients/*.hpp)
file(GLOB source_files
  ./../../../Source/ServiceClients/*.cpp)

add_library(ServiceClients ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(ServiceClients PROPERTIES OUTPUT_NAME
  nexus_service_clients)
install(TARGETS ServiceClients CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS ServiceClients CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
