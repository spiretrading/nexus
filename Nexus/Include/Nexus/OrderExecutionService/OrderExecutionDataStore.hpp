#ifndef NEXUS_ORDEREXECUTIONDATASTORE_HPP
#define NEXUS_ORDEREXECUTIONDATASTORE_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/OrderExecutionService/AccountOrderSubmissionEntry.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \struct OrderExecutionDataStore
      \brief Base class used to store Order execution data.
   */
  struct OrderExecutionDataStore : Beam::Concept<OrderExecutionDataStore> {

    //! Loads the initial Sequences to use for an account.
    /*!
      \param account The account to load the initial Sequences for.
      \return The initial Sequences to use for the <i>account</i>.
    */
    AccountOrderSubmissionEntry::InitialSequences LoadInitialSequences(
      const Beam::ServiceLocator::DirectoryEntry& account);

    //! Executes an Order submission query.
    /*!
      \param query The search query to execute.
      \return The list of SequencedOrderRecords satisfying the <i>query</i>.
    */
    std::vector<SequencedOrderRecord> LoadOrderSubmissions(
      const AccountQuery& query);

    //! Executes an ExecutionReport query.
    /*!
      \param query The search query to execute.
      \return The list of SequencedExecutionReports satisfying the <i>query</i>.
    */
    std::vector<SequencedExecutionReport> LoadExecutionReports(
      const AccountQuery& query);

    //! Stores a SequencedAccountOrderInfo.
    /*!
      \param orderInfo The SequencedAccountOrderInfo to store.
    */
    void Store(const SequencedAccountOrderInfo& orderInfo);

    //! Stores a SequencedAccountExecutionReport.
    /*!
      \param executionReport The SequencedAccountExecutionReport to store.
    */
    void Store(const SequencedAccountExecutionReport& executionReport);

    void Open();

    void Close();
  };
}
}

#endif
