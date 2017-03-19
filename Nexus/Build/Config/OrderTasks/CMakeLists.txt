file(GLOB header_files
  ./../../../Include/Nexus/OrderTasks/*.hpp)
file(GLOB source_files
  ./../../../Source/OrderTasks/*.cpp)

add_library(OrderTasks ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(OrderTasks PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
  OUTPUT_NAME nexus_order_tasks)
install(TARGETS OrderTasks CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderTasks CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
