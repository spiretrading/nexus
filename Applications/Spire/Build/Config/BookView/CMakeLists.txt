find_package(Qt5Widgets REQUIRED)

file(GLOB header_files
  ./../../../Include/Spire/BookView/*.hpp)
file(GLOB source_files
  ./../../../Source/BookView/*.cpp)
file(GLOB form_files
  ./../../../Source/BookView/*.ui)

qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)

source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(BookView
  ${header_files}
  ${source_files}
  ${generated_header_files}
  ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(BookView PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
  OUTPUT_NAME spire_book_view)
install(TARGETS BookView CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS BookView CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)
