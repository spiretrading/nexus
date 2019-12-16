include(
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Config/dependencies.cmake")
set(NEXUS_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(NEXUS_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Source")
set(BEAM_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Include")
set(BEAM_PYTHON_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Debug/_beam.so")
set(BEAM_PYTHON_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Release/_beam.so")
set(LUA_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.3.5/install/include")
set(LUA_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.3.5/install/lib/liblua.a")
set(LUA_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/lua-5.3.5/install/lib/liblua.a")
