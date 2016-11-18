find_package(Qt5Widgets REQUIRED)

file(GLOB header_files
  ./../../../Include/Spire/*.hpp)
file(GLOB source_files
  ./../../../Source/Spire/*.cpp)
file(GLOB form_files 
  ./../../../Source/Spire/*.ui)

qt5_wrap_ui(generated_header_files ${form_files})
find_cpp_qobject(qobject_source_files ${header_files})
qt5_wrap_cpp(generated_source_files ${qobject_source_files}
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)

source_group("Form Files" FILES ${form_files})
source_group("Generated Files\\Header Files" FILES ${generated_header_files})
source_group("Generated Files\\Source Files" FILES ${generated_source_files})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  set(app_type WIN32)
elseif(APPLE)
  set(app_type MACOSX_BUNDLE)
else()
  set(resource_files "")
endif(WIN32)

add_executable(Spire ${app_type} ${header_files} ${source_files}
  ${generated_header_files} ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(Spire Login Resources
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
  debug $ENV{QT_GUI_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_GUI_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_HBNG_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_HBNG_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_MAIN_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_MAIN_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_PCRE_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_PCRE_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_PLATFORM_SUPPORT_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_PLATFORM_SUPPORT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_SVG_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_SVG_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_WIDGETS_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{ZLIB_LIBRARY_DEBUG_PATH}
  optimized $ENV{ZLIB_LIBRARY_OPTIMIZED_PATH})

if(WIN32)
  target_link_libraries(Spire
    debug $ENV{QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized $ENV{QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    imm32.lib winmm.lib ws2_32.lib opengl32.lib)
endif()

if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  target_link_libraries(Spire
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_REGEX_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()

install(TARGETS Spire DESTINATION $ENV{INSTALL_DIRECTORY})
