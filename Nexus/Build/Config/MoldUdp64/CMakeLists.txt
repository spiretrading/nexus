file(GLOB header_files
  ./../../../Include/Nexus/MoldUdp64/*.hpp)
file(GLOB source_files
  ./../../../Source/MoldUdp64/*.cpp)

add_library(MoldUdp64 ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(MoldUdp64 PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_mold_udp64)
install(TARGETS MoldUdp64 CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS MoldUdp64 CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
