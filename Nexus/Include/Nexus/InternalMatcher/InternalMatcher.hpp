#ifndef NEXUS_INTERNALMATCHER_HPP
#define NEXUS_INTERNALMATCHER_HPP

namespace Nexus {
namespace InternalMatcher {
  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
    class InternalMatchingOrderExecutionDriver;
  struct MatchReportBuilder;
  class NullMatchReportBuilder;
}
}

#endif
