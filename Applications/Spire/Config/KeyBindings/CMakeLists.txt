find_package(Qt5Widgets REQUIRED)
file(GLOB header_files ${SPIRE_INCLUDE_PATH}/Spire/KeyBindings/*.hpp)
file(GLOB source_files ${SPIRE_SOURCE_PATH}/KeyBindings/*.cpp)
file(GLOB form_files ${SPIRE_SOURCE_PATH}/KeyBindings/*.ui)
qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(KeyBindings ${header_files} ${source_files}
  ${generated_header_files} ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(KeyBindings PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_key_bindings)
if(WIN32)
  add_custom_command(TARGET KeyBindings PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET KeyBindings PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
install(TARGETS KeyBindings CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS KeyBindings CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
