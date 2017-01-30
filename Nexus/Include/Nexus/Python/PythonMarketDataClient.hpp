#ifndef NEXUS_PYTHONMARKETDATACLIENT_HPP
#define NEXUS_PYTHONMARKETDATACLIENT_HPP
#include <memory>
#include <Beam/Python/Python.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/Python/Python.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace Python {

  /*! \class PythonMarketDataClient
      \brief Wraps a MarketDataClient so that it can be used from within Python.
   */
  class PythonMarketDataClient :
      public MarketDataService::WrapperMarketDataClient<
      std::unique_ptr<MarketDataService::VirtualMarketDataClient>> {
    public:

      //! Constructs a PythonMarketDataClient.
      /*!
        \param client The MarketDataClient to wrap.
      */
      PythonMarketDataClient(
        std::unique_ptr<MarketDataService::VirtualMarketDataClient> client);

      //! Submits a query for OrderImbalances.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for OrderImbalances.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for BboQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for BboQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for BookQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for BookQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for MarketQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for MarketQuotes.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for TimeAndSales.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);

      //! Submits a query for TimeAndSales.
      /*!
        \param query The query to submit.
        \param queue The Queue to store the results in.
      */
      void QuerySequencedTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::Python::PythonQueueWriter>& queue);
  };
}
}

#endif
