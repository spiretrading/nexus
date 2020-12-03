#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/OrderExecutionService/AccountOrderSubmissionEntry.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Base class used to store Order execution data. */
  struct OrderExecutionDataStore : Beam::Concept<OrderExecutionDataStore> {

    /**
     * Executes an Order submission query.
     * @param query The search query to execute.
     * @return The list of SequencedOrderRecords satisfying the <i>query</i>.
     */
    std::vector<SequencedOrderRecord> LoadOrderSubmissions(
      const AccountQuery& query);

    /**
     * Executes an ExecutionReport query.
     * @param query The search query to execute.
     * @return The list of SequencedExecutionReports satisfying the
     *         <i>query</i>.
     */
    std::vector<SequencedExecutionReport> LoadExecutionReports(
      const AccountQuery& query);

    /**
     * Stores a SequencedAccountOrderInfo.
     * @param orderInfo The SequencedAccountOrderInfo to store.
     */
    void Store(const SequencedAccountOrderInfo& orderInfo);

    /**
     * Stores a SequencedAccountExecutionReport.
     * @param executionReport The SequencedAccountExecutionReport to store.
     */
    void Store(const SequencedAccountExecutionReport& executionReport);

    void Close();
  };
}

#endif
