find_package(Qt5Widgets REQUIRED)

file(GLOB common_header_files
  ./../../../Include/Spire/Canvas/*.hpp
  ./../../../Include/Spire/Canvas/Common/*.hpp)
file(GLOB common_source_files
  ./../../../Source/Canvas/Common/*.cpp)
file(GLOB control_nodes_header_files
  ./../../../Include/Spire/Canvas/ControlNodes/*.hpp)
file(GLOB control_nodes_source_files
  ./../../../Source/Canvas/ControlNodes/*.cpp)
file(GLOB io_nodes_header_files
  ./../../../Include/Spire/Canvas/IONodes/*.hpp)
file(GLOB io_nodes_source_files
  ./../../../Source/Canvas/IONodes/*.cpp)
file(GLOB lua_nodes_header_files
  ./../../../Include/Spire/Canvas/LuaNodes/*.hpp)
file(GLOB lua_nodes_source_files
  ./../../../Source/Canvas/LuaNodes/*.cpp)
file(GLOB market_data_nodes_header_files
  ./../../../Include/Spire/Canvas/MarketDataNodes/*.hpp)
file(GLOB market_data_nodes_source_files
  ./../../../Source/Canvas/MarketDataNodes/*.cpp)
file(GLOB operations_header_files
  ./../../../Include/Spire/Canvas/Operations/*.hpp)
file(GLOB operations_source_files
  ./../../../Source/Canvas/Operations/*.cpp)
file(GLOB order_execution_nodes_header_files
  ./../../../Include/Spire/Canvas/OrderExecutionNodes/*.hpp)
file(GLOB order_execution_nodes_source_files
  ./../../../Source/Canvas/OrderExecutionNodes/*.cpp)
file(GLOB records_header_files
  ./../../../Include/Spire/Canvas/Records/*.hpp)
file(GLOB records_source_files
  ./../../../Source/Canvas/Records/*.cpp)
file(GLOB reference_nodes_header_files
  ./../../../Include/Spire/Canvas/ReferenceNodes/*.hpp)
file(GLOB reference_nodes_source_files
  ./../../../Source/Canvas/ReferenceNodes/*.cpp)
file(GLOB standard_nodes_header_files
  ./../../../Include/Spire/Canvas/StandardNodes/*.hpp)
file(GLOB standard_nodes_source_files
  ./../../../Source/Canvas/StandardNodes/*.cpp)
file(GLOB system_nodes_header_files
  ./../../../Include/Spire/Canvas/SystemNodes/*.hpp)
file(GLOB system_nodes_source_files
  ./../../../Source/Canvas/SystemNodes/*.cpp)
file(GLOB task_nodes_header_files
  ./../../../Include/Spire/Canvas/TaskNodes/*.hpp)
file(GLOB task_nodes_source_files
  ./../../../Source/Canvas/TaskNodes/*.cpp)
file(GLOB types_header_files
  ./../../../Include/Spire/Canvas/Types/*.hpp)
file(GLOB types_source_files
  ./../../../Source/Canvas/Types/*.cpp)
file(GLOB value_nodes_header_files
  ./../../../Include/Spire/Canvas/ValueNodes/*.hpp)
file(GLOB value_nodes_source_files
  ./../../../Source/Canvas/ValueNodes/*.cpp)
set(header_files
  ${common_header_files}
  ${control_nodes_header_files}
  ${io_nodes_header_files}
  ${lua_nodes_header_files}
  ${market_data_nodes_header_files}
  ${operations_header_files}
  ${order_execution_nodes_header_files}
  ${records_header_files}
  ${reference_nodes_header_files}
  ${standard_nodes_header_files}
  ${system_nodes_header_files}
  ${task_nodes_header_files}
  ${types_header_files}
  ${value_nodes_header_files})
set(source_files
  ${common_source_files}
  ${control_nodes_source_files}
  ${io_nodes_source_files}
  ${lua_nodes_source_files}
  ${market_data_nodes_source_files}
  ${operations_source_files}
  ${order_execution_nodes_source_files}
  ${records_source_files}
  ${reference_nodes_source_files}
  ${standard_nodes_source_files}
  ${system_nodes_source_files}
  ${task_nodes_source_files}
  ${types_source_files}
  ${value_nodes_source_files})

find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
source_group("Common\\Header Files" FILES ${common_header_files})
source_group("Common\\Source Files" FILES ${common_source_files})
source_group("Control Nodes\\Header Files" FILES ${control_nodes_header_files})
source_group("Control Nodes\\Source Files" FILES ${control_nodes_source_files})
source_group("IO Nodes\\Header Files" FILES ${io_nodes_header_files})
source_group("IO Nodes\\Source Files" FILES ${io_nodes_source_files})
source_group("Lua Nodes\\Header Files" FILES ${lua_nodes_header_files})
source_group("Lua Nodes\\Source Files" FILES ${lua_nodes_source_files})
source_group("Market Data Nodes\\Header Files"
  FILES ${market_data_nodes_header_files})
source_group("Market Data Nodes\\Source Files"
  FILES ${market_data_nodes_source_files})
source_group("Operations\\Header Files" FILES ${operations_header_files})
source_group("Operations\\Source Files" FILES ${operations_source_files})
source_group("Order Execution Nodes\\Header Files"
  FILES ${order_execution_nodes_header_files})
source_group("Order Execution Nodes\\Source Files"
  FILES ${order_execution_nodes_source_files})
source_group("Records\\Header Files" FILES ${records_header_files})
source_group("Records\\Source Files" FILES ${records_source_files})
source_group("Reference Nodes\\Header Files"
  FILES ${reference_nodes_header_files})
source_group("Reference Nodes\\Source Files"
  FILES ${reference_nodes_source_files})
source_group("Standard Nodes\\Header Files"
  FILES ${standard_nodes_header_files})
source_group("Standard Nodes\\Source Files"
  FILES ${standard_nodes_source_files})
source_group("System Nodes\\Header Files" FILES ${system_nodes_header_files})
source_group("System Nodes\\Source Files" FILES ${system_nodes_source_files})
source_group("Task Nodes\\Header Files" FILES ${task_nodes_header_files})
source_group("Task Nodes\\Source Files" FILES ${task_nodes_source_files})
source_group("Types\\Header Files" FILES ${types_header_files})
source_group("Types\\Source Files" FILES ${types_source_files})
source_group("Value Nodes\\Header Files" FILES ${value_nodes_header_files})
source_group("Value Nodes\\Source Files" FILES ${value_nodes_source_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Canvas
  ${header_files}
  ${source_files}
  ${generated_header_files}
  ${generated_source_files})

set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Canvas PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_canvas)
install(TARGETS Canvas CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Canvas CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Spire/CanvasTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/CanvasTests/*.cpp)

add_library(CanvasTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(CanvasTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_canvas_tests)
install(TARGETS CanvasTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS CanvasTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(CanvasTester
  ./../../../Source/CanvasTester/main.cpp)
target_link_libraries(CanvasTester CanvasTests Canvas
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{LUA_LIBRARY_DEBUG_PATH}
  optimized $ENV{LUA_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{YAML_LIBRARY_DEBUG_PATH}
  optimized $ENV{YAML_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_CORE_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_CORE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_GUI_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{ZLIB_LIBRARY_DEBUG_PATH}
  optimized $ENV{ZLIB_LIBRARY_OPTIMIZED_PATH})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
if(WIN32)
  target_link_libraries(CanvasTester
    debug $ENV{QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized $ENV{QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    imm32.lib winmm.lib ws2_32.lib opengl32.lib)
endif()
if(UNIX)
  target_link_libraries(CanvasTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(CanvasTester rt socket nsl)
endif()

add_custom_command(TARGET CanvasTester POST_BUILD COMMAND CanvasTester)
install(TARGETS CanvasTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS CanvasTester CONFIGURATIONS Release
  RelWithDebInfo DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
