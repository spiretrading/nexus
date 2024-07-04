find_package(Qt5Widgets REQUIRED)
file(GLOB data_types_header_files
  ${SPIRE_INCLUDE_PATH}/Spire/StyleParser/DataTypes/*.hpp)
file(GLOB
  data_types_source_files ${SPIRE_SOURCE_PATH}/StyleParser/DataTypes/*.cpp)
set(header_files ${data_types_header_files})
set(source_files ${data_types_source_files})
source_group("DataTypes\\Header Files" FILES ${data_types_header_files})
source_group("DataTypes\\Source Files" FILES ${data_types_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(StyleParser ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(StyleParser PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_style_parser)
install(TARGETS StyleParser CONFIGURATIONS Debug
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Debug)
install(TARGETS StyleParser CONFIGURATIONS Release
  DESTINATION ${SPIRE_LIBRARY_INSTALL_DIRECTORY}/Release)
file(GLOB test_source_files ${SPIRE_SOURCE_PATH}/StyleParserTests/*.cpp)
add_executable(StyleParserTests ${test_source_files})
target_link_libraries(StyleParserTests Styles StyleParser
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
if(UNIX)
  target_link_libraries(StyleParserTests
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
  target_link_libraries(StyleParserTests
    debug ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_ACCESSIBILITY_LIBRARY_OPTIMIZED_PATH}
    debug ${QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized ${QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    Crypt32.lib dwmapi.lib imm32.lib netapi32.lib opengl32.lib userenv.lib
    version.lib winmm.lib ws2_32.lib wtsapi32.lib)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(StyleParserTests rt socket nsl)
endif()
add_custom_command(TARGET StyleParserTests POST_BUILD COMMAND StyleParserTests)
install(TARGETS StyleParserTests CONFIGURATIONS Debug
  DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Debug)
install(TARGETS StyleParserTests CONFIGURATIONS Release
  RelWithDebInfo DESTINATION ${SPIRE_TESTS_INSTALL_DIRECTORY}/Release)
