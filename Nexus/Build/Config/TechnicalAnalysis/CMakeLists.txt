file(GLOB header_files
  ./../../../Include/Nexus/TechnicalAnalysis/*.hpp)
file(GLOB source_files
  ./../../../Source/TechnicalAnalysis/*.cpp)

add_library(TechnicalAnalysis ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(TechnicalAnalysis PROPERTIES
  OUTPUT_NAME nexus_technical_analysis)
install(TARGETS TechnicalAnalysis CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS TechnicalAnalysis CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
