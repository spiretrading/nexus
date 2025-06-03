find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE header_files ${SPIRE_INCLUDE_PATH}/Spire/Playback/*.hpp)
file(GLOB_RECURSE source_files ${SPIRE_SOURCE_PATH}/Playback/*.cpp)
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
add_library(Playback ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Playback PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_playback)
install(TARGETS Playback CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS Playback CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
file(GLOB_RECURSE header_files ../../Include/Spire/PlaybackUiTester/*.hpp)
file(GLOB_RECURSE source_files ../../Source/PlaybackUiTester/*.cpp)
if(WIN32)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
endif()
add_executable(PlaybackUiTester ${app_type} ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(UNIX)
  target_link_libraries(PlaybackUiTester
    debug ${QT_GIF_LIBRARY_DEBUG_PATH}
    optimized ${QT_GIF_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLATFORM_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLATFORM_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLUGIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLUGIN_LIBRARY_OPTIMIZED_PATH})
  if(EXISTS ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH})
    target_link_libraries(PlaybackUiTester
      debug ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH}
      optimized ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_OPTIMIZED_PATH})
  endif()
  target_link_libraries(PlaybackUiTester
    debug ${QT_SERVICE_SUPPORT_LIBRARY_DEBUG_PATH}
    optimized ${QT_SERVICE_SUPPORT_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_THEME_LIBRARY_DEBUG_PATH}
    optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_DBUS_LIBRARY_DEBUG_PATH}
    optimized ${QT_DBUS_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XML_LIBRARY_DEBUG_PATH}
    optimized ${QT_XML_LIBRARY_OPTIMIZED_PATH})
endif()
target_link_libraries(PlaybackUiTester Async Utilities Styles Library Resources
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
  target_link_libraries(PlaybackUiTester
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
if(UNIX)
  target_link_libraries(PlaybackUiTester
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
install(
  TARGETS PlaybackUiTester DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY})
file(GLOB_RECURSE header_files ../../Include/Spire/PlaybackTester/*.hpp)
file(GLOB_RECURSE source_files ../../Source/PlaybackTester/*.cpp)
add_executable(PlaybackTester ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(UNIX)
  target_link_libraries(PlaybackTester
    debug ${QT_GIF_LIBRARY_DEBUG_PATH}
    optimized ${QT_GIF_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLATFORM_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLATFORM_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XCB_PLUGIN_LIBRARY_DEBUG_PATH}
    optimized ${QT_XCB_PLUGIN_LIBRARY_OPTIMIZED_PATH})
  if(EXISTS ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH})
    target_link_libraries(PlaybackTester
      debug ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_DEBUG_PATH}
      optimized ${QT_LINUX_ACCESSIBILITY_SUPPORT_LIBRARY_OPTIMIZED_PATH})
  endif()
  target_link_libraries(PlaybackTester
    debug ${QT_SERVICE_SUPPORT_LIBRARY_DEBUG_PATH}
    optimized ${QT_SERVICE_SUPPORT_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_THEME_LIBRARY_DEBUG_PATH}
    optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_DBUS_LIBRARY_DEBUG_PATH}
    optimized ${QT_DBUS_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_XML_LIBRARY_DEBUG_PATH}
    optimized ${QT_XML_LIBRARY_OPTIMIZED_PATH})
endif()
target_link_libraries(PlaybackTester Async Utilities Library Resources Styles
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
  target_link_libraries(PlaybackTester
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
if(UNIX)
  target_link_libraries(PlaybackTester
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
add_custom_command(TARGET PlaybackTester POST_BUILD COMMAND PlaybackTester)
install(TARGETS PlaybackTester DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY})
