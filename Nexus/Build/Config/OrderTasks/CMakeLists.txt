file(GLOB header_files
  ./../../../Include/Nexus/OrderTasks/*.hpp)
file(GLOB source_files
  ./../../../Source/OrderTasks/*.cpp)

add_library(OrderTasks ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(OrderTasks PROPERTIES
  OUTPUT_NAME nexus_order_tasks)
install(TARGETS OrderTasks CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderTasks CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

#file(GLOB test_header_files
#  ./../../../Include/Nexus/OrderTasksTests/*.hpp)
#file(GLOB test_source_files
#  ./../../../Source/OrderTasksTests/*.cpp)

#add_library(OrderTasksTests
#  ${test_header_files} ${test_source_files})
#set_source_files_properties(${test_header_files}
#  PROPERTIES HEADER_FILE_ONLY TRUE)
#set_target_properties(OrderTasksTests PROPERTIES
#  OUTPUT_NAME nexus_order_tasks_tests)
#install(TARGETS OrderTasksTests CONFIGURATIONS Debug
#  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
#install(TARGETS OrderTasksTests CONFIGURATIONS Release RelWithDebInfo
#  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

#add_executable(OrderTasksTester
#  ./../../../Source/OrderTasksTester/main.cpp)
#target_link_libraries(OrderTasksTester OrderTasksTests
#  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
#  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH})

#if(UNIX)
#  target_link_libraries(OrderTasksTester
#    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_REGEX_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
#    debug $ENV{BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH}
#    optimized $ENV{BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH}
#    pthread rt)
#endif()
#if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
#  target_link_libraries(OrderTasksTester rt socket nsl)
#endif()

#add_custom_command(TARGET OrderTasksTester POST_BUILD
#  COMMAND OrderTasksTester)
#install(TARGETS OrderTasksTester CONFIGURATIONS Debug
#  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
#install(TARGETS OrderTasksTester CONFIGURATIONS Release
#  RelWithDebInfo DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
