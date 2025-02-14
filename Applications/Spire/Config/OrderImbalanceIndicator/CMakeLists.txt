find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE
  header_files ${SPIRE_INCLUDE_PATH}/Spire/OrderImbalanceIndicator/*.hpp)
file(GLOB_RECURSE
  source_files ${SPIRE_SOURCE_PATH}/OrderImbalanceIndicator/*.cpp)
file(GLOB_RECURSE form_files ${SPIRE_SOURCE_PATH}/OrderImbalanceIndicator/*.ui)
qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(OrderImbalanceIndicator ${header_files} ${source_files}
  ${generated_header_files} ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(OrderImbalanceIndicator PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_order_imbalance_indicator)
install(TARGETS OrderImbalanceIndicator CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS OrderImbalanceIndicator CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
