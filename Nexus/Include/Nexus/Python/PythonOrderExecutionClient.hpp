#ifndef NEXUS_PYTHONORDEREXECUTIONCLIENT_HPP
#define NEXUS_PYTHONORDEREXECUTIONCLIENT_HPP
#include <memory>
#include <Beam/Python/Python.hpp>
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/Python/Python.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace Python {

  /*! \class PythonOrderExecutionClient
      \brief Wraps an OrderExecutionClient so that it can be used from within
             Python.
   */
  class PythonOrderExecutionClient :
      public OrderExecutionService::WrapperOrderExecutionClient<
      std::unique_ptr<OrderExecutionService::VirtualOrderExecutionClient>> {
    public:

      //! Constructs a PythonOrderExecutionClient.
      /*!
        \param client The OrderExecutionClient to wrap.
      */
      PythonOrderExecutionClient(std::unique_ptr<
        OrderExecutionService::VirtualOrderExecutionClient> client);

      virtual ~PythonOrderExecutionClient();

      //! Submits a query for OrderRecords.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryOrderRecords(const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for Order submissions.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedOrderSubmissions(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for Order submissions.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryOrderSubmissions(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for ExecutionReports.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryExecutionReports(
        const OrderExecutionService::AccountQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);
  };
}
}

#endif
