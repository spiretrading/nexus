file(GLOB header_files
  ./../../../Include/Nexus/StampProtocol/*.hpp)
file(GLOB source_files
  ./../../../Source/StampProtocol/*.cpp)

add_library(StampProtocol ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(StampProtocol PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
  OUTPUT_NAME nexus_stamp_protocol)
install(TARGETS StampProtocol CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS StampProtocol CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
