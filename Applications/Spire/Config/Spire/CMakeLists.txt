find_package(Qt5Widgets REQUIRED)
file(GLOB header_files ${SPIRE_INCLUDE_PATH}/Spire/Spire/*.hpp)
file(GLOB source_files ${SPIRE_SOURCE_PATH}/Spire/*.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(WIN32)
  file(GLOB resource_files ${SPIRE_SOURCE_PATH}/Spire/*.rc)
  file(GLOB manifest_files ${SPIRE_SOURCE_PATH}/Spire/*.manifest)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
  set(resource_files "")
  set(manifest_files "")
else()
  set(resource_files "")
  set(manifest_files "")
endif(WIN32)
include_directories(${PROJECT_BINARY_DIR})
add_executable(Spire ${app_type} ${header_files} ${source_files}
  ${resource_files} ${manifest_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(Spire Async Dashboard Charting PortfolioViewer Utilities
  RiskTimer AccountViewer LegacyKeyBindings KeyBindings Blotter Catalog BookView
  Library Login Resources TimeAndSales OrderImbalanceIndicator InputWidgets
  CanvasView Canvas LegacyResources LegacyUI Toolbar
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${LUA_LIBRARY_DEBUG_PATH}
  optimized ${LUA_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_ACCESS_LIBRARY_DEBUG_PATH}
  optimized ${QT_ACCESS_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_CORE_LIBRARY_DEBUG_PATH}
  optimized ${QT_CORE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_EVENT_LIBRARY_DEBUG_PATH}
  optimized ${QT_EVENT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FONT_LIBRARY_DEBUG_PATH}
  optimized ${QT_FONT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FREETYPE_LIBRARY_DEBUG_PATH}
  optimized ${QT_FREETYPE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_GIF_LIBRARY_DEBUG_PATH}
  optimized ${QT_GIF_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_GUI_LIBRARY_DEBUG_PATH}
  optimized ${QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_HBNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_HBNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized ${QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PCRE_LIBRARY_DEBUG_PATH}
  optimized ${QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_PNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_SVG_ICON_PLUGIN_LIBRARY_DEBUG_PATH}
  optimized ${QT_SVG_ICON_PLUGIN_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_SVG_LIBRARY_DEBUG_PATH}
  optimized ${QT_SVG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_THEME_LIBRARY_DEBUG_PATH}
  optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized ${QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  target_link_libraries(Spire
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(WIN32)
  target_link_libraries(Spire
    debug ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib userenv.lib
    version.lib winmm.lib ws2_32.lib wtsapi32.lib)
endif()
install(TARGETS Spire DESTINATION ${PROJECT_BINARY_DIR}/Application)
