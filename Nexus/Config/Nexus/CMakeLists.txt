file(GLOB accounting_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Accounting/*.hpp)
source_group("Accounting" FILES ${accounting_header_files})
file(GLOB administration_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/AdministrationService/*.hpp)
source_group("AdministrationService"
  FILES ${administration_service_header_files})
file(GLOB backtester_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Backtester/*.hpp)
source_group("Backtester" FILES ${backtester_header_files})
file(GLOB binary_sequence_protocol_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/BinarySequenceProtocol/*.hpp)
source_group("BinarySequenceProtocol"
  FILES ${binary_sequence_protocol_header_files})
file(GLOB charting_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/ChartingService/*.hpp)
source_group("ChartingService" FILES ${charting_service_header_files})
file(GLOB compliance_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Compliance/*.hpp)
source_group("Compliance" FILES ${compliance_header_files})
file(GLOB definitions_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/Definitions/*.hpp)
source_group("Definitions" FILES ${definitions_header_files})
file(GLOB definitions_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/DefinitionsService/*.hpp)
source_group("DefinitionsService" FILES ${definitions_service_header_files})
file(GLOB fee_handling_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/FeeHandling/*.hpp)
source_group("FeeHandling" FILES ${fee_handling_header_files})
file(GLOB fix_utilities_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/FixUtilities/*.hpp)
source_group("FixUtilities" FILES ${fix_utilities_header_files})
file(GLOB internal_matcher_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/InternalMatcher/*.hpp)
source_group("InternalMatcher" FILES ${internal_matcher_header_files})
file(GLOB market_data_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/MarketDataService/*.hpp)
source_group("MarketDataService" FILES ${market_data_service_header_files})
file(GLOB mold_udp64_header_files ${NEXUS_INCLUDE_PATH}/Nexus/MoldUdp64/*.hpp)
source_group("MoldUdp64" FILES ${mold_udp64_header_files})
file(GLOB order_execution_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/OrderExecutionService/*.hpp)
source_group("OrderExecutionService"
  FILES ${order_execution_service_header_files})
file(GLOB parsers_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Parsers/*.hpp)
source_group("Parsers" FILES ${parsers_header_files})
file(GLOB queries_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Queries/*.hpp)
source_group("Queries" FILES ${queries_header_files})
file(GLOB risk_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/RiskService/*.hpp)
source_group("RiskService" FILES ${risk_service_header_files})
file(GLOB service_clients_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/ServiceClients/*.hpp)
source_group("ServiceClients" FILES ${service_clients_header_files})
file(GLOB simulation_matcher_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/SimulationMatcher/*.hpp)
source_group("SimulationMatcher" FILES ${simulation_matcher_header_files})
file(GLOB soup_bin_tcp_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/SoupBinTcp/*.hpp)
source_group("SoupBinTcp" FILES ${soup_bin_tcp_header_files})
file(GLOB stamp_protocol_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/StampProtocol/*.hpp)
source_group("StampProtocol" FILES ${stamp_protocol_header_files})
file(GLOB technical_analysis_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/TechnicalAnalysis/*.hpp)
source_group("TechnicalAnalysis" FILES ${technical_analysis_header_files})
file(GLOB header_files ${accounting_header_files}
  ${administration_service_header_files} ${backtester_header_files}
  ${binary_sequence_protocol_header_files} ${charting_service_header_files}
  ${compliance_header_files} ${definitions_header_files}
  ${definitions_service_header_files} ${fee_handling_header_files}
  ${fix_utilities_header_files} ${internal_matcher_header_files}
  ${market_data_service_header_files} ${mold_udp64_header_files}
  ${order_execution_service_header_files} ${parsers_header_files}
  ${queries_header_files} ${risk_service_header_files}
  ${service_clients_header_files} ${simulation_matcher_header_files}
  ${soup_bin_tcp_header_files} ${stamp_protocol_header_files}
  ${technical_analysis_header_files} )
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_library(Nexus ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Nexus PROPERTIES STATIC_LIBRARY_FLAGS_RELEASE
  "${CMAKE_LIBRARY_FLAGS}" LINKER_LANGUAGE CXX OUTPUT_NAME nexus)
if(WIN32)
  add_custom_command(TARGET Nexus PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET Nexus PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
