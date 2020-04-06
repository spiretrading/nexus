#ifndef NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataClient for use with Python.
   * param <C> The type of MarketDataClient to wrap.
   */
  template<typename C>
  class ToPythonMarketDataClient final : public VirtualMarketDataClient {
    public:

      //! The type of MarketDataClient to wrap.
      using Client = C;

      //! Constructs a ToPythonMarketDataClient.
      /*!
        \param client The MarketDataClient to wrap.
      */
      ToPythonMarketDataClient(std::unique_ptr<Client> client);

      ~ToPythonMarketDataClient() override;

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue) override;

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue)
        override;

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue)
        override;

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) override;

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedBookQuote>>& queue) override;

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) override;

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedMarketQuote>>& queue) override;

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) override;

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedTimeAndSale>>& queue) override;

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) override;

      SecuritySnapshot LoadSecuritySnapshot(const Security& security) override;

      SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override;

      boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) override;

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonMarketDataClient.
   * @param client The MarketDataClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonMarketDataClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonMarketDataClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonMarketDataClient<C>::ToPythonMarketDataClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonMarketDataClient<C>::~ToPythonMarketDataClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename C>
  SecuritySnapshot ToPythonMarketDataClient<C>::LoadSecuritySnapshot(
      const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename C>
  SecurityTechnicals ToPythonMarketDataClient<C>::
      LoadSecurityTechnicals(const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename C>
  boost::optional<SecurityInfo> ToPythonMarketDataClient<C>::LoadSecurityInfo(
      const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecurityInfo(security);
  }

  template<typename C>
  std::vector<SecurityInfo> ToPythonMarketDataClient<C>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::Open() {
    auto release = Beam::Python::GilRelease();
    m_client->Open();
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
