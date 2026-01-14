include(
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Config/dependencies.cmake")
set(NEXUS_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(NEXUS_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Source")
set(BEAM_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Include")
set(BEAM_PYTHON_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Debug/beam.so")
set(BEAM_PYTHON_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Release/beam.so")
set(LUA_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.5.0/install/include")
set(LUA_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.5.0/install/lib/liblua.a")
set(LUA_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.5.0/install/lib/liblua.a")
set(QUICK_FIX_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/quickfix-v.1.15.1/include")
set(QUICK_FIX_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/quickfix-v.1.15.1/lib/libquickfix.a")
set(QUICK_FIX_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/quickfix-v.1.15.1/lib/libquickfix.a")
