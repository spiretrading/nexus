find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE common_header_files ${SPIRE_INCLUDE_PATH}/Spire/Canvas/*.hpp
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/Common/*.hpp)
file(GLOB_RECURSE common_source_files ${SPIRE_SOURCE_PATH}/Canvas/Common/*.cpp)
file(GLOB_RECURSE control_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/ControlNodes/*.hpp)
file(GLOB_RECURSE control_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/ControlNodes/*.cpp)
file(GLOB_RECURSE io_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/IONodes/*.hpp)
file(GLOB_RECURSE
  io_nodes_source_files ${SPIRE_SOURCE_PATH}/Canvas/IONodes/*.cpp)
file(GLOB_RECURSE lua_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/LuaNodes/*.hpp)
file(GLOB_RECURSE
  lua_nodes_source_files ${SPIRE_SOURCE_PATH}/Canvas/LuaNodes/*.cpp)
file(GLOB_RECURSE market_data_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/MarketDataNodes/*.hpp)
file(GLOB_RECURSE market_data_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/MarketDataNodes/*.cpp)
file(GLOB_RECURSE operations_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/Operations/*.hpp)
file(GLOB_RECURSE
  operations_source_files ${SPIRE_SOURCE_PATH}/Canvas/Operations/*.cpp)
file(GLOB_RECURSE order_execution_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/OrderExecutionNodes/*.hpp)
file(GLOB_RECURSE order_execution_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/OrderExecutionNodes/*.cpp)
file(GLOB_RECURSE
  records_header_files ${SPIRE_INCLUDE_PATH}/Spire/Canvas/Records/*.hpp)
file(GLOB_RECURSE
  records_source_files ${SPIRE_SOURCE_PATH}/Canvas/Records/*.cpp)
file(GLOB_RECURSE reference_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/ReferenceNodes/*.hpp)
file(GLOB_RECURSE reference_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/ReferenceNodes/*.cpp)
file(GLOB_RECURSE standard_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/StandardNodes/*.hpp)
file(GLOB_RECURSE standard_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/StandardNodes/*.cpp)
file(GLOB_RECURSE system_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/SystemNodes/*.hpp)
file(GLOB_RECURSE system_nodes_source_files
  ${SPIRE_SOURCE_PATH}/Canvas/SystemNodes/*.cpp)
file(GLOB_RECURSE
  tasks_header_files ${SPIRE_INCLUDE_PATH}/Spire/Canvas/Tasks/*.hpp)
file(GLOB_RECURSE tasks_source_files ${SPIRE_SOURCE_PATH}/Canvas/Tasks/*.cpp)
file(GLOB_RECURSE
  types_header_files ${SPIRE_INCLUDE_PATH}/Spire/Canvas/Types/*.hpp)
file(GLOB_RECURSE types_source_files ${SPIRE_SOURCE_PATH}/Canvas/Types/*.cpp)
file(GLOB_RECURSE value_nodes_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/Canvas/ValueNodes/*.hpp)
file(GLOB_RECURSE
  value_nodes_source_files ${SPIRE_SOURCE_PATH}/Canvas/ValueNodes/*.cpp)
set(header_files ${common_header_files} ${control_nodes_header_files}
  ${io_nodes_header_files} ${lua_nodes_header_files}
  ${market_data_nodes_header_files} ${operations_header_files}
  ${order_execution_nodes_header_files} ${records_header_files}
  ${reference_nodes_header_files} ${standard_nodes_header_files}
  ${system_nodes_header_files} ${tasks_header_files}
  ${types_header_files} ${value_nodes_header_files})
set(source_files ${common_source_files} ${control_nodes_source_files}
  ${io_nodes_source_files} ${lua_nodes_source_files}
  ${market_data_nodes_source_files} ${operations_source_files}
  ${order_execution_nodes_source_files} ${records_source_files}
  ${reference_nodes_source_files} ${standard_nodes_source_files}
  ${system_nodes_source_files} ${tasks_source_files}
  ${types_source_files} ${value_nodes_source_files})
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
source_group("Tasks\\Header Files" FILES ${tasks_header_files})
source_group("Tasks\\Source Files" FILES ${tasks_source_files})
source_group("Types\\Header Files" FILES ${types_header_files})
source_group("Types\\Source Files" FILES ${types_source_files})
source_group("Value Nodes\\Header Files" FILES ${value_nodes_header_files})
source_group("Value Nodes\\Source Files" FILES ${value_nodes_source_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Canvas ${header_files} ${source_files} ${generated_header_files}
  ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Canvas PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_canvas)
install(TARGETS Canvas CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Canvas CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
file(GLOB test_source_files ${SPIRE_SOURCE_PATH}/CanvasTests/*.cpp)
add_executable(CanvasTests ${test_source_files})
target_link_libraries(CanvasTests Async Dashboard Charting PortfolioViewer
  Utilities RiskTimer AccountViewer Blotter Catalog BookView TimeAndSales
  OrderImbalanceIndicator InputWidgets CanvasView Canvas KeyBindings Styles
  Library LegacyResources LegacyUI Toolbar
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${LUA_LIBRARY_DEBUG_PATH}
  optimized ${LUA_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_CORE_LIBRARY_DEBUG_PATH}
  optimized ${QT_CORE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FREETYPE_LIBRARY_DEBUG_PATH}
  optimized ${QT_FREETYPE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_HBNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_HBNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_GUI_LIBRARY_DEBUG_PATH}
  optimized ${QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized ${QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PCRE_LIBRARY_DEBUG_PATH}
  optimized ${QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized ${QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_PNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_EVENT_LIBRARY_DEBUG_PATH}
  optimized ${QT_EVENT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FONT_LIBRARY_DEBUG_PATH}
  optimized ${QT_FONT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_THEME_LIBRARY_DEBUG_PATH}
  optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_ACCESS_LIBRARY_DEBUG_PATH}
  optimized ${QT_ACCESS_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(CanvasTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(WIN32)
  target_link_libraries(CanvasTests
    debug ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib userenv.lib
    version.lib winmm.lib ws2_32.lib wtsapi32.lib)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(CanvasTests rt socket nsl)
endif()
add_custom_command(TARGET CanvasTests POST_BUILD COMMAND CanvasTests)
install(TARGETS CanvasTests CONFIGURATIONS Debug
  DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Debug)
install(TARGETS CanvasTests CONFIGURATIONS Release
  RelWithDebInfo DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Release)
