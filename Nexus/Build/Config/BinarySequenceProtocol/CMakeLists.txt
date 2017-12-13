file(GLOB header_files
  ./../../../Include/Nexus/BinarySequenceProtocol/*.hpp)
file(GLOB source_files
  ./../../../Source/BinarySequenceProtocol/*.cpp)

add_library(BinarySequenceProtocol ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(BinarySequenceProtocol PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_binary_sequence_protocol)
install(TARGETS BinarySequenceProtocol CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS BinarySequenceProtocol CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
