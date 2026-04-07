file(GLOB_RECURSE
  accounting_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Accounting/*.hpp)
source_group("Accounting" FILES ${accounting_header_files})
file(GLOB_RECURSE administration_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/AdministrationService/*.hpp)
source_group("AdministrationService"
  FILES ${administration_service_header_files})
file(GLOB_RECURSE
  backtester_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Backtester/*.hpp)
source_group("Backtester" FILES ${backtester_header_files})
file(GLOB_RECURSE charting_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/ChartingService/*.hpp)
source_group("ChartingService" FILES ${charting_service_header_files})
file(GLOB_RECURSE
  clients_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Clients/*.hpp)
source_group("Clients" FILES ${clients_header_files})
file(GLOB_RECURSE
  compliance_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Compliance/*.hpp)
source_group("Compliance" FILES ${compliance_header_files})
file(GLOB_RECURSE definitions_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/Definitions/*.hpp)
source_group("Definitions" FILES ${definitions_header_files})
file(GLOB_RECURSE definitions_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/DefinitionsService/*.hpp)
source_group("DefinitionsService" FILES ${definitions_service_header_files})
file(GLOB_RECURSE fee_handling_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/FeeHandling/*.hpp)
source_group("FeeHandling" FILES ${fee_handling_header_files})
file(GLOB_RECURSE fix_utilities_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/FixUtilities/*.hpp)
source_group("FixUtilities" FILES ${fix_utilities_header_files})
file(GLOB_RECURSE internal_matcher_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/InternalMatcher/*.hpp)
source_group("InternalMatcher" FILES ${internal_matcher_header_files})
file(GLOB_RECURSE market_data_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/MarketDataService/*.hpp)
source_group("MarketDataService" FILES ${market_data_service_header_files})
file(GLOB_RECURSE
  mold_udp64_header_files ${NEXUS_INCLUDE_PATH}/Nexus/MoldUdp64/*.hpp)
source_group("MoldUdp64" FILES ${mold_udp64_header_files})
file(GLOB_RECURSE order_execution_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/OrderExecutionService/*.hpp)
source_group("OrderExecutionService"
  FILES ${order_execution_service_header_files})
file(GLOB_RECURSE
  parsers_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Parsers/*.hpp)
source_group("Parsers" FILES ${parsers_header_files})
file(GLOB_RECURSE
  queries_header_files ${NEXUS_INCLUDE_PATH}/Nexus/Queries/*.hpp)
source_group("Queries" FILES ${queries_header_files})
file(GLOB_RECURSE risk_service_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/RiskService/*.hpp)
source_group("RiskService" FILES ${risk_service_header_files})
file(GLOB_RECURSE simulation_matcher_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/SimulationMatcher/*.hpp)
source_group("SimulationMatcher" FILES ${simulation_matcher_header_files})
file(GLOB_RECURSE soup_bin_tcp_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/SoupBinTcp/*.hpp)
source_group("SoupBinTcp" FILES ${soup_bin_tcp_header_files})
file(GLOB_RECURSE stamp_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/Stamp/*.hpp)
source_group("Stamp" FILES ${stamp_header_files})
file(GLOB_RECURSE technical_analysis_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/TechnicalAnalysis/*.hpp)
source_group("TechnicalAnalysis" FILES ${technical_analysis_header_files})
file(GLOB_RECURSE test_environment_header_files
  ${NEXUS_INCLUDE_PATH}/Nexus/TestEnvironment/*.hpp)
source_group("TestEnvironment" FILES ${test_environment_header_files})
file(GLOB_RECURSE header_files ${accounting_header_files}
  ${administration_service_header_files} ${backtester_header_files}
  ${charting_service_header_files} ${clients_header_files}
  ${compliance_header_files} ${definitions_header_files}
  ${definitions_service_header_files} ${fee_handling_header_files}
  ${fix_utilities_header_files} ${internal_matcher_header_files}
  ${market_data_service_header_files} ${mold_udp64_header_files}
  ${order_execution_service_header_files} ${parsers_header_files}
  ${queries_header_files} ${risk_service_header_files}
  ${simulation_matcher_header_files} ${soup_bin_tcp_header_files}
  ${stamp_header_files} ${technical_analysis_header_files}
  ${test_environment_header_files})
add_library(Nexus ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Nexus PROPERTIES LINKER_LANGUAGE CXX OUTPUT_NAME nexus)
