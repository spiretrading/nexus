find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE header_files ${SPIRE_INCLUDE_PATH}/Spire/Blotter/*.hpp)
file(GLOB_RECURSE source_files ${SPIRE_SOURCE_PATH}/Blotter/*.cpp)
file(GLOB_RECURSE form_files ${SPIRE_SOURCE_PATH}/Blotter/*.ui)
qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Blotter ${header_files} ${source_files} ${generated_header_files}
  ${generated_source_files})
beam_configure_target(Blotter)
target_link_libraries(Blotter BeamRoutines)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Blotter PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_blotter)
beam_install_target(Blotter "${SPIRE_LIBRARY_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE header_files ../../Include/Spire/BlotterUiTester/*.hpp)
file(GLOB_RECURSE source_files ../../Source/BlotterUiTester/*.cpp)
if(WIN32)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
endif()
add_executable(BlotterUiTester ${app_type} ${header_files} ${source_files})
beam_configure_target(BlotterUiTester)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(BlotterUiTester Blotter Library Styles Resources
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(WIN32)
  target_link_libraries(BlotterUiTester
    debug ${QT_GIF_LIBRARY_DEBUG_PATH}
    optimized ${QT_GIF_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_SVG_ICON_PLUGIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_SVG_ICON_PLUGIN_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_SVG_LIBRARY_DEBUG_PATH}
    optimized ${QT_SVG_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WIDGETS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_FONT_LIBRARY_DEBUG_PATH}
    optimized ${QT_FONT_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_GUI_LIBRARY_DEBUG_PATH}
    optimized ${QT_GUI_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_PNG_LIBRARY_DEBUG_PATH}
    optimized ${QT_PNG_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_EVENT_LIBRARY_DEBUG_PATH}
    optimized ${QT_EVENT_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_ACCESS_LIBRARY_DEBUG_PATH}
    optimized ${QT_ACCESS_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_HBNG_LIBRARY_DEBUG_PATH}
    optimized ${QT_HBNG_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_CORE_LIBRARY_DEBUG_PATH}
    optimized ${QT_CORE_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_PCRE_LIBRARY_DEBUG_PATH}
    optimized ${QT_PCRE_LIBRARY_OPTIMIZED_PATH})
  target_link_libraries(BlotterUiTester
    debug ${QT_THEME_LIBRARY_DEBUG_PATH}
    optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_FREETYPE_LIBRARY_DEBUG_PATH}
    optimized ${QT_FREETYPE_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_MAIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_MAIN_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    dwmapi.lib imm32.lib netapi32.lib opengl32.lib Userenv.lib
    version.lib winmm.lib ws2_32.lib Wtsapi32.lib)
endif()
beam_install_target(BlotterUiTester "${SPIRE_TESTS_INSTALL_DIRECTORY}")
