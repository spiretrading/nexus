file(GLOB header_files
  ./../../../Include/Nexus/SimulationMatcher/*.hpp)
file(GLOB source_files
  ./../../../Source/SimulationMatcher/*.cpp)

add_library(SimulationMatcher ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(SimulationMatcher PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME nexus_simulation_matcher)
install(TARGETS SimulationMatcher CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS SimulationMatcher CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
