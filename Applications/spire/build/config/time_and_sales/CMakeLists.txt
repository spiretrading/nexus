find_package(Qt5Widgets REQUIRED)

file(GLOB header_files ./../../../include/spire/time_and_sales/*.hpp)
file(GLOB source_files ./../../../source/time_and_sales/*.cpp)

add_library(time_and_sales ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(time_and_sales PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_time_and_sales)
install(TARGETS time_and_sales CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS time_and_sales CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
