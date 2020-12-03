#ifndef NEXUS_ACCOUNT_QUERY_HPP
#define NEXUS_ACCOUNT_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/Range.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

namespace Nexus::OrderExecutionService {
  using SequencedOrder = Beam::Queries::SequencedValue<const Order*>;
  using SequencedOrderInfo = Beam::Queries::SequencedValue<OrderInfo>;
  using SequencedOrderRecord = Beam::Queries::SequencedValue<OrderRecord>;
  using SequencedExecutionReport =
    Beam::Queries::SequencedValue<ExecutionReport>;
  using AccountOrderInfo = Beam::Queries::IndexedValue<OrderInfo,
    Beam::ServiceLocator::DirectoryEntry>;
  using AccountOrderRecord = Beam::Queries::IndexedValue<OrderRecord,
    Beam::ServiceLocator::DirectoryEntry>;
  using AccountExecutionReport = Beam::Queries::IndexedValue<ExecutionReport,
    Beam::ServiceLocator::DirectoryEntry>;
  using SequencedAccountOrderInfo =
    Beam::Queries::SequencedValue<AccountOrderInfo>;
  using SequencedAccountOrderRecord =
    Beam::Queries::SequencedValue<AccountOrderRecord>;
  using SequencedAccountExecutionReport = Beam::Queries::SequencedValue<
    AccountExecutionReport>;

  /** Defines the type of query used to receive Order submissions. */
  using AccountQuery =
    Beam::Queries::BasicQuery<Beam::ServiceLocator::DirectoryEntry>;
}

namespace Beam::Queries {
  template<>
  struct TimestampAccessor<Nexus::OrderExecutionService::OrderRecord> {
    const boost::posix_time::ptime& operator ()(
        const Nexus::OrderExecutionService::OrderRecord& value) const {
      return value.m_info.m_timestamp;
    }

    boost::posix_time::ptime& operator ()(
        Nexus::OrderExecutionService::OrderRecord& value) const {
      return value.m_info.m_timestamp;
    }
  };
}

#endif
