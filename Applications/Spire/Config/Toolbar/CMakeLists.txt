find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE header_files ${SPIRE_INCLUDE_PATH}/Spire/Toolbar/*.hpp)
file(GLOB_RECURSE source_files ${SPIRE_SOURCE_PATH}/Toolbar/*.cpp)
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(Toolbar ${header_files} ${source_files})
beam_configure_target(Toolbar)
target_link_libraries(Toolbar BeamRoutines)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Toolbar PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_toolbar)
beam_install_target(Toolbar "${SPIRE_LIBRARY_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE header_files ../../Include/Spire/ToolbarUiTester/*.hpp)
file(GLOB_RECURSE source_files ../../Source/ToolbarUiTester/*.cpp)
if(WIN32)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
endif()
add_executable(ToolbarUiTester ${app_type} ${header_files} ${source_files})
beam_configure_target(ToolbarUiTester)
target_link_libraries(ToolbarUiTester BeamRoutines)
target_compile_definitions(ToolbarUiTester PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(ToolbarUiTester Async Dashboard Charting PortfolioViewer
  Utilities RiskTimer KeyBindings Blotter Catalog BookView Library
  Resources Styles TimeAndSales OrderImbalanceIndicator InputWidgets
  CanvasView Canvas LegacyResources LegacyUI Toolbar
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
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(WIN32)
  target_link_libraries(ToolbarUiTester
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
  target_link_libraries(ToolbarUiTester
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
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib Userenv.lib
    version.lib winmm.lib ws2_32.lib Wtsapi32.lib)
endif()
beam_install_target(ToolbarUiTester "${SPIRE_TESTS_INSTALL_DIRECTORY}")
