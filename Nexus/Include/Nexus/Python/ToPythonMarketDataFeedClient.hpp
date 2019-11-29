#ifndef NEXUS_TO_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TO_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataFeedClient.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataClient for use with Python.
   * @param <ClientType> The type of MarketDataClient to wrap.
   */
  template<typename ClientType>
  class ToPythonMarketDataFeedClient final :
      public VirtualMarketDataFeedClient {
    public:

      /** The type of MarketDataFeedClient to wrap. */
      using Client = ClientType;

      /**
       * Constructs a ToPythonMarketDataFeedClient.
       * @param client The MarketDataFeedClient to wrap.
       */
      ToPythonMarketDataFeedClient(std::unique_ptr<Client> client);

      ~ToPythonMarketDataFeedClient() override;

      void Add(const SecurityInfo& securityInfo) override;

      void PublishOrderImbalance(
        const MarketOrderImbalance& orderImbalance) override;

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

      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
      Beam::IO::OpenState m_openState;

      void Shutdown();
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

  template<typename ClientType>
  ToPythonMarketDataFeedClient<ClientType>::ToPythonMarketDataFeedClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename ClientType>
  ToPythonMarketDataFeedClient<ClientType>::~ToPythonMarketDataFeedClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock(gil);
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::Add(
      const SecurityInfo& securityInfo) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Add(securityInfo);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->PublishOrderImbalance(orderImbalance);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::PublishBboQuote(
      const SecurityBboQuote& bboQuote) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->PublishBboQuote(bboQuote);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->PublishMarketQuote(marketQuote);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::SetBookQuote(
      const SecurityBookQuote& bookQuote) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->SetBookQuote(bookQuote);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::AddOrder(
      const Security& security, MarketCode market,
      const std::string& mpid, bool isPrimaryMpid, const std::string& id,
      Side side, Money price, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side, price,
      size, timestamp);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::ModifyOrderSize(
      const std::string& id, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->ModifyOrderSize(id, size, timestamp);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::OffsetOrderSize(
      const std::string& id, Quantity delta,
      const boost::posix_time::ptime& timestamp) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->OffsetOrderSize(id, delta, timestamp);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::ModifyOrderPrice(
      const std::string& id, Money price,
      const boost::posix_time::ptime& timestamp) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->ModifyOrderPrice(id, price, timestamp);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::DeleteOrder(
      const std::string& id, const boost::posix_time::ptime& timestamp) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->DeleteOrder(id, timestamp);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->PublishTimeAndSale(timeAndSale);
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::Open() {
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
  void ToPythonMarketDataFeedClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonMarketDataFeedClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}

#endif
