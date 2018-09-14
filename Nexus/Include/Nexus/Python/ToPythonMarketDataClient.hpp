#ifndef NEXUS_TO_PYTHON_MARKET_DATA_CLIENT_HPP
#define NEXUS_TO_PYTHON_MARKET_DATA_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ToPythonMarketDataClient
      \brief Wraps a MarketDataClient for use with Python.
      \tparam ClientType The type of MarketDataClient to wrap.
   */
  template<typename ClientType>
  class ToPythonMarketDataClient final : public VirtualMarketDataClient {
    public:

      //! The type of MarketDataClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonMarketDataClient.
      /*!
        \param client The MarketDataClient to wrap.
      */
      ToPythonMarketDataClient(std::unique_ptr<Client> client);

      virtual ~ToPythonMarketDataClient() override;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue) override;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue)
        override;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue)
        override;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) override;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedBookQuote>>& queue) override;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) override;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedMarketQuote>>& queue) override;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) override;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedTimeAndSale>>& queue) override;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) override;

      virtual SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) override;

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override;

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      std::unique_ptr<Client> m_client;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonMarketDataClient.
  /*!
    \param client The MarketDataClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonMarketDataClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonMarketDataClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonMarketDataClient<ClientType>::ToPythonMarketDataClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonMarketDataClient<ClientType>::~ToPythonMarketDataClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename ClientType>
  SecuritySnapshot ToPythonMarketDataClient<ClientType>::LoadSecuritySnapshot(
      const Security& security) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename ClientType>
  SecurityTechnicals ToPythonMarketDataClient<ClientType>::
      LoadSecurityTechnicals(const Security& security) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename ClientType>
  std::vector<SecurityInfo> ToPythonMarketDataClient<ClientType>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonMarketDataClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}
}

#endif
