find_package(Qt5Widgets REQUIRED)
file(GLOB_RECURSE header_files ${SPIRE_INCLUDE_PATH}/Spire/SignIn/*.hpp)
file(GLOB_RECURSE source_files ${SPIRE_SOURCE_PATH}/SignIn/*.cpp)
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(SignIn ${header_files} ${source_files})
beam_configure_target(SignIn)
target_link_libraries(SignIn BeamRoutines)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(SignIn PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME spire_sign_in)
beam_install_target(SignIn "${SPIRE_LIBRARY_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE test_header_files ../../Include/Spire/SignInUiTester/*.hpp)
file(GLOB_RECURSE test_source_files ../../Source/SignInUiTester/*.cpp)
if(WIN32)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
endif()
add_executable(
  SignInUiTester ${app_type} ${test_header_files} ${test_source_files})
beam_configure_target(SignInUiTester)
target_link_libraries(SignInUiTester BeamRoutines)
target_compile_definitions(SignInUiTester PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(
  ${test_header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(SignInUiTester Styles Library SignIn Resources
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
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
  target_link_libraries(SignInUiTester
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
  target_link_libraries(SignInUiTester
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
beam_install_target(SignInUiTester "${SPIRE_TESTS_INSTALL_DIRECTORY}")
