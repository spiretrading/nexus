file(GLOB styles_header_files ../../Include/Spire/Styles/*.hpp)
file(GLOB styles_source_files ../../Source/Styles/*.cpp)
source_group("Styles\\Header Files" FILES ${styles_header_files})
source_group("Styles\\Source Files" FILES ${styles_source_files})
file(GLOB header_files ${styles_header_files})
file(GLOB source_files ${styles_source_files})
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_library(Library ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Library PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" OUTPUT_NAME Spire)
install(TARGETS Library CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Library CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
