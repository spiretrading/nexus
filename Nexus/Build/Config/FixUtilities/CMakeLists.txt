file(GLOB header_files
  ./../../../Include/Nexus/FixUtilities/*.hpp)
file(GLOB source_files
  ./../../../Source/FixUtilities/*.cpp)

add_library(FixUtilities ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(FixUtilities PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_fix_utilities)
install(TARGETS FixUtilities CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS FixUtilities CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
