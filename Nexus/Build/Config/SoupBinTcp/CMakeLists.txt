file(GLOB header_files
  ./../../../Include/Nexus/SoupBinTcp/*.hpp)
file(GLOB source_files
  ./../../../Source/SoupBinTcp/*.cpp)

add_library(SoupBinTcp ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(SoupBinTcp PROPERTIES OUTPUT_NAME nexus_soup_bin_tcp)
install(TARGETS SoupBinTcp CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS SoupBinTcp CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
