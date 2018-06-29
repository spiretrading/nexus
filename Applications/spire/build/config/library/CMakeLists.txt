file(GLOB book_view_header_files ./../../../include/spire/book_view/*.hpp)
file(GLOB book_view_source_files ./../../../source/book_view/*.cpp)
source_group("Header Files\\book_view" FILES ${book_view_header_files})
source_group("Source Files\\book_view" FILES ${book_view_source_files})
file(GLOB common_header_files ./../../../include/spire/spire/*.hpp)
file(GLOB common_source_files ./../../../source/common/*.cpp)
source_group("Header Files\\common" FILES ${common_header_files})
source_group("Source Files\\common" FILES ${common_source_files})
file(GLOB login_header_files ./../../../include/spire/login/*.hpp)
file(GLOB login_source_files ./../../../source/login/*.cpp)
source_group("Header Files\\login" FILES ${login_header_files})
source_group("Source Files\\login" FILES ${login_source_files})
file(GLOB security_input_header_files
  ./../../../include/spire/security_input/*.hpp)
file(GLOB security_input_source_files ./../../../source/security_input/*.cpp)
source_group("Header Files\\security_input"
  FILES ${security_input_header_files})
source_group("Source Files\\security_input"
  FILES ${security_input_source_files})
file(GLOB time_and_sales_header_files
  ./../../../include/spire/time_and_sales/*.hpp)
file(GLOB time_and_sales_source_files ./../../../source/time_and_sales/*.cpp)
source_group("Header Files\\time_and_sales"
  FILES ${time_and_sales_header_files})
source_group("Source Files\\time_and_sales"
  FILES ${time_and_sales_source_files})
file(GLOB toolbar_header_files ./../../../include/spire/toolbar/*.hpp)
file(GLOB toolbar_source_files ./../../../source/toolbar/*.cpp)
source_group("Header Files\\toolbar" FILES ${toolbar_header_files})
source_group("Source Files\\toolbar" FILES ${toolbar_source_files})
file(GLOB ui_header_files ./../../../include/spire/ui/*.hpp)
file(GLOB ui_source_files ./../../../source/ui/*.cpp)
source_group("Header Files\\ui" FILES ${ui_header_files})
source_group("Source Files\\ui" FILES ${ui_source_files})
file(GLOB header_files ${book_view_header_files} ${common_header_files}
  ${login_header_files} ${security_input_header_files}
  ${time_and_sales_header_files} ${toolbar_header_files} ${ui_header_files})
file(GLOB source_files ${book_view_source_files} ${common_source_files}
  ${login_source_files} ${security_input_source_files}
  ${time_and_sales_source_files} ${toolbar_source_files} ${ui_source_files})

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
