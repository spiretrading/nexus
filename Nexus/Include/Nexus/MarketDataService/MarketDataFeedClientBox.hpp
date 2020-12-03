#ifndef NEXUS_MARKET_DATA_FEED_CLIENT_BOX_HPP
#define NEXUS_MARKET_DATA_FEED_CLIENT_BOX_HPP
#include <memory>
#include <string>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {

  /** Provides a generic interface over an arbitrary MarketDataFeedClient. */
  class MarketDataFeedClientBox {
    public:

      /**
       * Constructs a MarketDataFeedClientBox of a specified type using
       * emplacement.
       * @param <T> The type of market data feed client to emplace.
       * @param args The arguments to pass to the emplaced market data feed
       *        client.
       */
      template<typename T, typename... Args>
      explicit MarketDataFeedClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a MarketDataFeedClientBox by copying an existing market data
       * feed client.
       * @param client The client to copy.
       */
      template<typename MarketDataFeedClient>
      explicit MarketDataFeedClientBox(MarketDataFeedClient client);

      explicit MarketDataFeedClientBox(MarketDataFeedClientBox* client);

      explicit MarketDataFeedClientBox(
        const std::shared_ptr<MarketDataFeedClientBox>& client);

      explicit MarketDataFeedClientBox(
        const std::unique_ptr<MarketDataFeedClientBox>& client);

      void Add(const SecurityInfo& securityInfo);

      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance);

      void PublishBboQuote(const SecurityBboQuote& bboQuote);

      void PublishMarketQuote(const SecurityMarketQuote& marketQuote);

      void SetBookQuote(const SecurityBookQuote& bookQuote);

      void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);

      void ModifyOrderSize(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);

      void OffsetOrderSize(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);

      void ModifyOrderPrice(const std::string& id, Money price,
        boost::posix_time::ptime timestamp);

      void DeleteOrder(const std::string& id,
        boost::posix_time::ptime timestamp);

      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale);

      void Close();

    private:
      struct VirtualMarketDataFeedClient {
        virtual ~VirtualMarketDataFeedClient() = default;
        virtual void Add(const SecurityInfo& securityInfo) = 0;
        virtual void PublishOrderImbalance(
          const MarketOrderImbalance& orderImbalance) = 0;
        virtual void PublishBboQuote(const SecurityBboQuote& bboQuote) = 0;
        virtual void PublishMarketQuote(
          const SecurityMarketQuote& marketQuote) = 0;
        virtual void SetBookQuote(const SecurityBookQuote& bookQuote) = 0;
        virtual void AddOrder(const Security& security, MarketCode market,
          const std::string& mpid, bool isPrimaryMpid, const std::string& id,
          Side side, Money price, Quantity size,
          boost::posix_time::ptime timestamp) = 0;
        virtual void ModifyOrderSize(const std::string& id, Quantity size,
          boost::posix_time::ptime timestamp) = 0;
        virtual void OffsetOrderSize(const std::string& id, Quantity delta,
          boost::posix_time::ptime timestamp) = 0;
        virtual void ModifyOrderPrice(const std::string& id, Money price,
          boost::posix_time::ptime timestamp) = 0;
        virtual void DeleteOrder(const std::string& id,
          boost::posix_time::ptime timestamp) = 0;
        virtual void PublishTimeAndSale(
          const SecurityTimeAndSale& timeAndSale) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedMarketDataFeedClient final : VirtualMarketDataFeedClient {
        using MarketDataFeedClient = C;
        Beam::GetOptionalLocalPtr<MarketDataFeedClient> m_client;

        template<typename... Args>
        WrappedMarketDataFeedClient(Args&&... args);
        void Add(const SecurityInfo& securityInfo) override;
        void PublishOrderImbalance(
          const MarketOrderImbalance& orderImbalance) override;
        void PublishBboQuote(const SecurityBboQuote& bboQuote) override;
        void PublishMarketQuote(
          const SecurityMarketQuote& marketQuote) override;
        void SetBookQuote(const SecurityBookQuote& bookQuote) override;
        void AddOrder(const Security& security, MarketCode market,
          const std::string& mpid, bool isPrimaryMpid, const std::string& id,
          Side side, Money price, Quantity size,
          boost::posix_time::ptime timestamp) override;
        void ModifyOrderSize(const std::string& id, Quantity size,
          boost::posix_time::ptime timestamp) override;
        void OffsetOrderSize(const std::string& id, Quantity delta,
          boost::posix_time::ptime timestamp) override;
        void ModifyOrderPrice(const std::string& id, Money price,
          boost::posix_time::ptime timestamp) override;
        void DeleteOrder(const std::string& id,
          boost::posix_time::ptime timestamp) override;
        void PublishTimeAndSale(
          const SecurityTimeAndSale& timeAndSale) override;
        void Close() override;
      };
      std::shared_ptr<VirtualMarketDataFeedClient> m_client;
  };

  template<typename T, typename... Args>
  MarketDataFeedClientBox::MarketDataFeedClientBox(
    std::in_place_type_t<T>, Args&&... args)
      : m_client(std::make_shared<WrappedMarketDataFeedClient<T>>(
          std::forward<Args>(args)...)) {}

  template<typename MarketDataFeedClient>
  MarketDataFeedClientBox::MarketDataFeedClientBox(MarketDataFeedClient client)
    : MarketDataFeedClientBox(std::in_place_type<MarketDataFeedClient>,
        std::move(client)) {}

  inline MarketDataFeedClientBox::MarketDataFeedClientBox(
    MarketDataFeedClientBox* client)
      : MarketDataFeedClientBox(*client) {}

  inline MarketDataFeedClientBox::MarketDataFeedClientBox(
    const std::shared_ptr<MarketDataFeedClientBox>& client)
      : MarketDataFeedClientBox(*client) {}

  inline MarketDataFeedClientBox::MarketDataFeedClientBox(
    const std::unique_ptr<MarketDataFeedClientBox>& client)
      : MarketDataFeedClientBox(*client) {}

  inline void MarketDataFeedClientBox::Add(const SecurityInfo& securityInfo) {
    return m_client->Add(securityInfo);
  }

  inline void MarketDataFeedClientBox::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance) {
    return m_client->PublishOrderImbalance(orderImbalance);
  }

  inline void MarketDataFeedClientBox::PublishBboQuote(
      const SecurityBboQuote& bboQuote) {
    return m_client->PublishBboQuote(bboQuote);
  }

  inline void MarketDataFeedClientBox::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote) {
    return m_client->PublishMarketQuote(marketQuote);
  }

  inline void MarketDataFeedClientBox::SetBookQuote(
      const SecurityBookQuote& bookQuote) {
    return m_client->SetBookQuote(bookQuote);
  }

  inline void MarketDataFeedClientBox::AddOrder(const Security& security,
      MarketCode market, const std::string& mpid, bool isPrimaryMpid,
      const std::string& id, Side side, Money price, Quantity size,
      boost::posix_time::ptime timestamp) {
    return m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side,
      price, size, timestamp);
  }

  inline void MarketDataFeedClientBox::ModifyOrderSize(const std::string& id,
      Quantity size, boost::posix_time::ptime timestamp) {
    return m_client->ModifyOrderSize(id, size, timestamp);
  }

  inline void MarketDataFeedClientBox::OffsetOrderSize(const std::string& id,
      Quantity delta, boost::posix_time::ptime timestamp) {
    return m_client->OffsetOrderSize(id, delta, timestamp);
  }

  inline void MarketDataFeedClientBox::ModifyOrderPrice(const std::string& id,
      Money price, boost::posix_time::ptime timestamp) {
    return m_client->ModifyOrderPrice(id, price, timestamp);
  }

  inline void MarketDataFeedClientBox::DeleteOrder(const std::string& id,
      boost::posix_time::ptime timestamp) {
    return m_client->DeleteOrder(id, timestamp);
  }

  inline void MarketDataFeedClientBox::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale) {
    return m_client->PublishTimeAndSale(timeAndSale);
  }

  inline void MarketDataFeedClientBox::Close() {
    return m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::
    WrappedMarketDataFeedClient(Args&&... args)
      : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Add(
      const SecurityInfo& securityInfo) {
    return m_client->Add(securityInfo);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::
      PublishOrderImbalance(const MarketOrderImbalance& orderImbalance) {
    return m_client->PublishOrderImbalance(orderImbalance);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::PublishBboQuote(
      const SecurityBboQuote& bboQuote) {
    return m_client->PublishBboQuote(bboQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::
      PublishMarketQuote(const SecurityMarketQuote& marketQuote) {
    return m_client->PublishMarketQuote(marketQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::SetBookQuote(
      const SecurityBookQuote& bookQuote) {
    return m_client->SetBookQuote(bookQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::AddOrder(
      const Security& security, MarketCode market, const std::string& mpid,
      bool isPrimaryMpid, const std::string& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    return m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side,
      price, size, timestamp);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::ModifyOrderSize(
      const std::string& id, Quantity size,
      boost::posix_time::ptime timestamp) {
    return m_client->ModifyOrderSize(id, size, timestamp);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::OffsetOrderSize(
      const std::string& id, Quantity delta,
      boost::posix_time::ptime timestamp) {
    return m_client->OffsetOrderSize(id, delta, timestamp);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::
      ModifyOrderPrice(const std::string& id, Money price,
        boost::posix_time::ptime timestamp) {
    return m_client->ModifyOrderPrice(id, price, timestamp);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::DeleteOrder(
      const std::string& id, boost::posix_time::ptime timestamp) {
    return m_client->DeleteOrder(id, timestamp);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::
      PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale) {
    return m_client->PublishTimeAndSale(timeAndSale);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
