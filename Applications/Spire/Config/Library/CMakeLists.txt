file(GLOB book_view_header_files ../../Include/Spire/BookView/*.hpp)
file(GLOB book_view_source_files ../../Source/BookView/*.cpp)
source_group("BookView\\Header Files" FILES ${book_view_header_files})
source_group("BookView\\Source Files" FILES ${book_view_source_files})
file(GLOB charting_header_files ../../Include/Spire/Charting/*.hpp)
file(GLOB charting_source_files ../../Source/Charting/*.cpp)
source_group("Charting\\Header Files" FILES ${charting_header_files})
source_group("Charting\\Source Files" FILES ${charting_source_files})
file(GLOB common_header_files ../../Include/Spire/Spire/*.hpp)
file(GLOB common_source_files ../../Source/Common/*.cpp)
source_group("Common\\Header Files" FILES ${common_header_files})
source_group("Common\\Source Files" FILES ${common_source_files})
file(GLOB key_bindings_header_files ../../Include/Spire/KeyBindings/*.hpp)
file(GLOB key_bindings_source_files ../../Source/KeyBindings/*.cpp)
source_group("KeyBindings\\Header Files" FILES ${key_bindings_header_files})
source_group("KeyBindings\\Source Files" FILES ${key_bindings_source_files})
file(GLOB login_header_files ../../Include/Spire/Login/*.hpp)
file(GLOB login_source_files ../../Source/Login/*.cpp)
source_group("Login\\Header Files" FILES ${login_header_files})
source_group("Login\\Source Files" FILES ${login_source_files})
file(GLOB order_imbalance_indicator_header_files
  ../../Include/Spire/OrderImbalanceIndicator/*.hpp)
file(GLOB order_imbalance_indicator_source_files
  ../../Source/OrderImbalanceIndicator/*.cpp)
source_group("OrderImbalanceIndicator\\Header Files"
  FILES ${order_imbalance_indicator_header_files})
source_group("OrderImbalanceIndicator\\Source Files"
  FILES ${order_imbalance_indicator_source_files})
file(GLOB security_input_header_files ../../Include/Spire/SecurityInput/*.hpp)
file(GLOB security_input_source_files ../../Source/SecurityInput/*.cpp)
source_group("SecurityInput\\Header Files" FILES ${security_input_header_files})
source_group("SecurityInput\\Source Files" FILES ${security_input_source_files})
file(GLOB styles_header_files ../../Include/Spire/Styles/*.hpp)
file(GLOB styles_source_files ../../Source/Styles/*.cpp)
source_group("Styles\\Header Files" FILES ${styles_header_files})
source_group("Styles\\Source Files" FILES ${styles_source_files})
file(GLOB time_and_sales_header_files ../../Include/Spire/TimeAndSales/*.hpp)
file(GLOB time_and_sales_source_files ../../Source/TimeAndSales/*.cpp)
source_group("TimeAndSales\\Header Files" FILES ${time_and_sales_header_files})
source_group("TimeAndSales\\Source Files" FILES ${time_and_sales_source_files})
file(GLOB toolbar_header_files ../../Include/Spire/Toolbar/*.hpp)
file(GLOB toolbar_source_files ../../Source/Toolbar/*.cpp)
source_group("Toolbar\\Header Files" FILES ${toolbar_header_files})
source_group("Toolbar\\Source Files" FILES ${toolbar_source_files})
file(GLOB ui_header_files ../../Include/Spire/Ui/*.hpp)
file(GLOB ui_source_files ../../Source/Ui/*.cpp)
source_group("Ui\\Header Files" FILES ${ui_header_files})
source_group("Ui\\Source Files" FILES ${ui_source_files})
file(GLOB header_files ${book_view_header_files} ${charting_header_files}
  ${common_header_files} ${key_bindings_header_files} ${login_header_files}
  ${order_imbalance_indicator_header_files} ${security_input_header_files}
  ${styles_header_files} ${time_and_sales_header_files} ${toolbar_header_files}
  ${ui_header_files})
file(GLOB source_files ${book_view_source_files} ${charting_source_files}
  ${common_source_files} ${key_bindings_source_files} ${login_source_files}
  ${order_imbalance_indicator_source_files} ${security_input_source_files}
  ${styles_source_files} ${time_and_sales_source_files} ${toolbar_source_files}
  ${ui_source_files})
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
