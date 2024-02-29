find_package(Qt5Widgets REQUIRED)
file(GLOB header_files ${SPIRE_INCLUDE_PATH}/Spire/CanvasView/*.hpp)
file(GLOB source_files ${SPIRE_SOURCE_PATH}/CanvasView/*.cpp)
file(GLOB form_files ${SPIRE_SOURCE_PATH}/CanvasView/*.ui)
qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(CanvasView ${header_files} ${source_files}
  ${generated_header_files} ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(CanvasView PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_canvas_view)
install(TARGETS CanvasView CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS CanvasView CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
file(GLOB test_source_files ${SPIRE_SOURCE_PATH}/CanvasViewTests/*.cpp)
add_executable(CanvasViewTests ${test_source_files})
target_link_libraries(CanvasViewTests Async Dashboard Charting PortfolioViewer
  Utilities RiskTimer AccountViewer LegacyKeyBindings Blotter Catalog BookView
  TimeAndSales OrderImbalanceIndicator InputWidgets CanvasView Canvas
  Library LegacyResources LegacyUI Toolbar
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${LUA_LIBRARY_DEBUG_PATH}
  optimized ${LUA_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_CORE_LIBRARY_DEBUG_PATH}
  optimized ${QT_CORE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FREETYPE_LIBRARY_DEBUG_PATH}
  optimized ${QT_FREETYPE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_HBNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_HBNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_GUI_LIBRARY_DEBUG_PATH}
  optimized ${QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized ${QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PCRE_LIBRARY_DEBUG_PATH}
  optimized ${QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized ${QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_PNG_LIBRARY_DEBUG_PATH}
  optimized ${QT_PNG_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_EVENT_LIBRARY_DEBUG_PATH}
  optimized ${QT_EVENT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_FONT_LIBRARY_DEBUG_PATH}
  optimized ${QT_FONT_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_THEME_LIBRARY_DEBUG_PATH}
  optimized ${QT_THEME_LIBRARY_OPTIMIZED_PATH}
  debug ${QT_ACCESS_LIBRARY_DEBUG_PATH}
  optimized ${QT_ACCESS_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
if(UNIX)
  target_link_libraries(CanvasViewTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(WIN32)
  target_link_libraries(CanvasViewTests
    debug ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib userenv.lib
    version.lib winmm.lib ws2_32.lib wtsapi32.lib)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(CanvasViewTests rt socket nsl)
endif()
add_custom_command(TARGET CanvasViewTests POST_BUILD COMMAND CanvasViewTests)
install(TARGETS CanvasViewTests CONFIGURATIONS Debug
  DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Debug)
install(TARGETS CanvasViewTests CONFIGURATIONS Release
  RelWithDebInfo DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Release)
