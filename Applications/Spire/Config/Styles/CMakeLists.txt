file(GLOB_RECURSE header_files ../../Include/Spire/Styles/*.hpp)
file(GLOB_RECURSE source_files ../../Source/Styles/*.cpp)
source_group("Header Files" FILES ${header_files})
source_group("Source Files" FILES ${source_files})
add_library(Styles ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Styles PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" OUTPUT_NAME styles)
install(TARGETS Library CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Library CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
file(GLOB_RECURSE test_header_files ../../Include/Spire/StylesTester/*.hpp)
file(GLOB_RECURSE test_source_files ../../Source/StylesTester/*.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(StylesTester ${test_header_files} ${test_source_files})
set_source_files_properties(
  ${test_header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(UNIX)
  target_link_libraries(StylesTester
    debug ${QT_GIF_LIBRARY_DEBUG_PATH}
    optimized ${QT_GIF_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLATFORM_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLATFORM_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLUGIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLUGIN_LIBRARY_OPTIMIZED_PATH})
  if(EXISTS ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH})
    target_link_libraries(StylesTester
      debug ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH}
      optimized ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_OPTIMIZED_PATH})
  endif()
  target_link_libraries(StylesTester
    debug ${QT_SERVICE_SUPPORT_LIBRARY_DEBUG_PATH}
    optimized ${QT_SERVICE_SUPPORT_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_THEME_LIBRARY_DEBUG_PATH}
    optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_DBUS_LIBRARY_DEBUG_PATH}
    optimized ${QT_DBUS_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XML_LIBRARY_DEBUG_PATH}
    optimized ${QT_XML_LIBRARY_OPTIMIZED_PATH})
endif()
target_link_libraries(StylesTester Styles Library Resources
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
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
  optimized ${QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(WIN32)
  target_link_libraries(StylesTester
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
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib Userenv.lib version.lib
    winmm.lib ws2_32.lib Wtsapi32.lib)
endif()
if(UNIX)
  target_link_libraries(StylesTester
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_REGEX_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_EDID_LIBRARY_DEBUG_PATH}
    optimized ${QT_EDID_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLATFORM_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLATFORM_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLUGIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLUGIN_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_LIBRARY_OPTIMIZED_PATH}
    dbus-1 dl fontconfig freetype glib-2.0 ICE double-conversion pthread rt SM
    X11 X11-xcb xcb Xi xkbcommon xkbcommon-x11 Xrender)
endif()
add_custom_command(TARGET StylesTester POST_BUILD COMMAND StylesTester)
install(TARGETS StylesTester DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY})
