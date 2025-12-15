#ifndef NEXUS_ACCOUNT_QUERY_HPP
#define NEXUS_ACCOUNT_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/Range.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

namespace Nexus {
  using SequencedOrder = Beam::SequencedValue<std::shared_ptr<Order>>;
  using SequencedOrderInfo = Beam::SequencedValue<OrderInfo>;
  using SequencedOrderRecord = Beam::SequencedValue<OrderRecord>;
  using SequencedExecutionReport = Beam::SequencedValue<ExecutionReport>;
  using AccountOrderInfo = Beam::IndexedValue<OrderInfo, Beam::DirectoryEntry>;
  using AccountOrderRecord =
    Beam::IndexedValue<OrderRecord, Beam::DirectoryEntry>;
  using AccountExecutionReport =
    Beam::IndexedValue<ExecutionReport, Beam::DirectoryEntry>;
  using SequencedAccountOrderInfo = Beam::SequencedValue<AccountOrderInfo>;
  using SequencedAccountOrderRecord = Beam::SequencedValue<AccountOrderRecord>;
  using SequencedAccountExecutionReport =
    Beam::SequencedValue<AccountExecutionReport>;

  /** Defines the type of query used to receive Order submissions. */
  using AccountQuery = Beam::BasicQuery<Beam::DirectoryEntry>;
}

namespace Beam {
  template<>
  struct TimestampAccessor<Nexus::OrderRecord> {
    const boost::posix_time::ptime& operator ()(
        const Nexus::OrderRecord& value) const {
      return value.m_info.m_timestamp;
    }

    boost::posix_time::ptime& operator ()(Nexus::OrderRecord& value) const {
      return value.m_info.m_timestamp;
    }
  };
}

#endif
