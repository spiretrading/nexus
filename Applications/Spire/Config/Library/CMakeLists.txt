file(GLOB_RECURSE common_header_files ../../Include/Spire/Spire/*.hpp)
file(GLOB_RECURSE common_source_files ../../Source/Common/*.cpp)
source_group("Common\\Header Files" FILES ${common_header_files})
source_group("Common\\Source Files" FILES ${common_source_files})
file(GLOB_RECURSE ui_header_files ../../Include/Spire/Ui/*.hpp)
file(GLOB_RECURSE ui_source_files ../../Source/Ui/*.cpp)
source_group("Ui\\Header Files" FILES ${ui_header_files})
source_group("Ui\\Source Files" FILES ${ui_source_files})
file(GLOB_RECURSE header_files ${common_header_files} ${ui_header_files})
file(GLOB_RECURSE source_files ${common_source_files} ${ui_source_files})
add_library(Library ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Library PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" OUTPUT_NAME Spire)
install(TARGETS Library CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Library CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
