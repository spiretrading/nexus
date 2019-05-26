file(GLOB book_view_header_files ./../../../include/spire/book_view/*.hpp)
file(GLOB book_view_source_files ./../../../source/book_view/*.cpp)
source_group("book_view\\Header Files" FILES ${book_view_header_files})
source_group("book_view\\Source Files" FILES ${book_view_source_files})

file(GLOB charting_header_files ./../../../include/spire/charting/*.hpp)
file(GLOB charting_source_files ./../../../source/charting/*.cpp)
source_group("charting\\Header Files" FILES ${charting_header_files})
source_group("charting\\Source Files" FILES ${charting_source_files})


file(GLOB common_header_files ./../../../include/spire/spire/*.hpp)
file(GLOB common_source_files ./../../../source/common/*.cpp)
source_group("common\\Header Files" FILES ${common_header_files})
source_group("common\\Source Files" FILES ${common_source_files})
file(GLOB key_bindings_header_files ./../../../include/spire/key_bindings/*.hpp)
file(GLOB key_bindings_source_files ./../../../source/key_bindings/*.cpp)
source_group("key_bindings\\Header Files" FILES ${key_bindings_header_files})
source_group("key_bindings\\Source Files" FILES ${key_bindings_source_files})
file(GLOB login_header_files ./../../../include/spire/login/*.hpp)
file(GLOB login_source_files ./../../../source/login/*.cpp)
source_group("login\\Header Files" FILES ${login_header_files})
source_group("login\\Source Files" FILES ${login_source_files})
file(GLOB security_input_header_files
  ./../../../include/spire/security_input/*.hpp)
file(GLOB security_input_source_files ./../../../source/security_input/*.cpp)
source_group("security_input\\Header Files"
  FILES ${security_input_header_files})
source_group("security_input\\Source Files"
  FILES ${security_input_source_files})
file(GLOB time_and_sales_header_files
  ./../../../include/spire/time_and_sales/*.hpp)
file(GLOB time_and_sales_source_files ./../../../source/time_and_sales/*.cpp)
source_group("time_and_sales\\Header Files"
  FILES ${time_and_sales_header_files})
source_group("time_and_sales\\Source Files"
  FILES ${time_and_sales_source_files})
file(GLOB toolbar_header_files ./../../../include/spire/toolbar/*.hpp)
file(GLOB toolbar_source_files ./../../../source/toolbar/*.cpp)
source_group("toolbar\\Header Files" FILES ${toolbar_header_files})
source_group("toolbar\\Source Files" FILES ${toolbar_source_files})
file(GLOB ui_header_files ./../../../include/spire/ui/*.hpp)
file(GLOB ui_source_files ./../../../source/ui/*.cpp)
source_group("ui\\Header Files" FILES ${ui_header_files})
source_group("ui\\Source Files" FILES ${ui_source_files})
file(GLOB header_files ${book_view_header_files} ${charting_header_files}
  ${common_header_files} ${key_bindings_header_files} ${login_header_files}
  ${security_input_header_files} ${time_and_sales_header_files}
  ${toolbar_header_files} ${ui_header_files})
file(GLOB source_files ${book_view_source_files} ${charting_source_files}
  ${common_source_files} ${key_bindings_source_files} ${login_source_files}
  ${security_input_source_files} ${time_and_sales_source_files}
  ${toolbar_source_files} ${ui_source_files})

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_library(library ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(library PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" OUTPUT_NAME spire)
install(TARGETS library CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/debug)
install(TARGETS library CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/release)
