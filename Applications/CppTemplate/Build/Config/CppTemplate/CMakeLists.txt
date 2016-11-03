find_package(Qt5Widgets REQUIRED)

file(GLOB header_files
  ./../../../Include/CppTemplate/*.hpp)
file(GLOB source_files
  ./../../../Source/*.cpp)
file(GLOB form_files 
  ./../../../Source/*.ui)

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

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

add_executable(CppTemplate ${app_type} ${header_files} ${source_files} ${generated_header_files} ${generated_source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(CppTemplate   
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
  debug $ENV{QT_WIDGETS_LIBRARY_DEBUG_PATH}
  optimized $ENV{QT_WIDGETS_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(CppTemplate)
endif(UNIX)
if(WIN32)
  target_link_libraries(CppTemplate
    debug $ENV{QT_WINDOWS_LIBRARY_DEBUG_PATH}
    optimized $ENV{QT_WINDOWS_LIBRARY_OPTIMIZED_PATH}
    imm32.lib winmm.lib ws2_32.lib opengl32.lib)
endif()

install(TARGETS CppTemplate DESTINATION $ENV{INSTALL_DIRECTORY})
