include(
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Config/dependencies.cmake")
set(CATCH_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Catch2-2.2.1/single_include")
set(NEXUS_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(NEXUS_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Source")
set(BEAM_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Include")
set(BEAM_PYTHON_LIBRARY_DEBUG_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Debug/beam.so")
set(BEAM_PYTHON_LIBRARY_OPTIMIZED_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Beam/Beam/Libraries/Release/beam.so")
