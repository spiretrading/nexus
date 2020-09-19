#ifndef NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/VirtualMarketDataFeedClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataFeedClient for use with Python.
   * param <C> The type of MarketDataFeedClient to wrap.
   */
  template<typename C>
  class ToPythonMarketDataFeedClient final :
      public VirtualMarketDataFeedClient {
    public:

      //! The type of MarketDataFeedClient to wrap.
      using Client = C;

      //! Constructs a ToPythonMarketDataFeedClient.
      /*!
        \param client The MarketDataFeedClient to wrap.
      */
      ToPythonMarketDataFeedClient(std::unique_ptr<Client> client);

      ~ToPythonMarketDataFeedClient() override;

      void Add(const SecurityInfo& securityInfo) override;

      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance)
        override;

      void PublishBboQuote(const SecurityBboQuote& bboQuote) override;

      void PublishMarketQuote(const SecurityMarketQuote& marketQuote) override;

      void SetBookQuote(const SecurityBookQuote& bookQuote) override;

      void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp) override;

      void ModifyOrderSize(const std::string& id, Quantity size,
        const boost::posix_time::ptime& timestamp) override;

      void OffsetOrderSize(const std::string& id, Quantity delta,
        const boost::posix_time::ptime& timestamp) override;

      void ModifyOrderPrice(const std::string& id, Money price,
        const boost::posix_time::ptime& timestamp) override;

      void DeleteOrder(const std::string& id,
        const boost::posix_time::ptime& timestamp) override;

      void PublishTimeAndSale(
        const SecurityTimeAndSale& timeAndSale) override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonMarketDataFeedClient.
   * @param client The MarketDataFeedClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonMarketDataFeedClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonMarketDataFeedClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonMarketDataFeedClient<C>::ToPythonMarketDataFeedClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonMarketDataFeedClient<C>::~ToPythonMarketDataFeedClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Add(const SecurityInfo& securityInfo) {
    auto release = Beam::Python::GilRelease();
    m_client->Add(securityInfo);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance) {
    auto release = Beam::Python::GilRelease();
    m_client->PublishOrderImbalance(orderImbalance);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::PublishBboQuote(
      const SecurityBboQuote& bboQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->PublishBboQuote(bboQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->PublishMarketQuote(marketQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::SetBookQuote(
      const SecurityBookQuote& bookQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->SetBookQuote(bookQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::AddOrder(const Security& security,
      MarketCode market, const std::string& mpid, bool isPrimaryMpid,
      const std::string& id, Side side, Money price, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side, price,
      size, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::ModifyOrderSize(const std::string& id,
      Quantity size, const boost::posix_time::ptime& timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->ModifyOrderSize(id, size, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::OffsetOrderSize(const std::string& id,
      Quantity delta, const boost::posix_time::ptime& timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->OffsetOrderSize(id, delta, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::ModifyOrderPrice(const std::string& id,
      Money price, const boost::posix_time::ptime& timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->ModifyOrderPrice(id, price, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::DeleteOrder(const std::string& id,
      const boost::posix_time::ptime& timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->DeleteOrder(id, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale) {
    auto release = Beam::Python::GilRelease();
    m_client->PublishTimeAndSale(timeAndSale);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
