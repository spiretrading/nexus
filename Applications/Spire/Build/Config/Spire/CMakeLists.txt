find_package(Qt5Widgets REQUIRED)

file(GLOB header_files
  ./../../../Include/Spire/Spire/*.hpp)
file(GLOB source_files
  ./../../../Source/Spire/*.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  file(GLOB resource_files
    ./../../../Source/Spire/*.rc)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
else()
  set(resource_files "")
endif(WIN32)

add_executable(Spire ${app_type} ${header_files} ${source_files}
  ${resource_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(Spire Dashboard Charting PortfolioViewer Utilities
  RiskTimer AccountViewer KeyBindings Blotter Catalog BookView TimeAndSales
  OrderImbalanceIndicator InputWidgets CanvasView Canvas UI Resources
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{LUA_LIBRARY_DEBUG_PATH}
  optimized $ENV{LUA_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized $ENV{OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized $ENV{OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{YAML_LIBRARY_DEBUG_PATH}
  optimized $ENV{YAML_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_CORE_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_CORE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_FREETYPE_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_FREETYPE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_HBNG_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_HBNG_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_GUI_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_PCRE_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_PNG_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_PNG_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_EVENT_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_EVENT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_FONT_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_FONT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_THEME_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_THEME_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_ACCESS_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_ACCESS_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{ZLIB_LIBRARY_DEBUG_PATH}
  optimized $ENV{ZLIB_LIBRARY_OPTIMIZED_PATH})

if(WIN32)
  target_link_libraries(Spire
    debug $ENV{QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized $ENV{QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    imm32.lib winmm.lib ws2_32.lib opengl32.lib version.lib netapi32.lib
    userenv.lib dwmapi.lib)
endif()

if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  target_link_libraries(Spire
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()

install(TARGETS Spire DESTINATION $ENV{INSTALL_DIRECTORY})
