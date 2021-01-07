#ifndef NEXUS_MARKET_DATA_FEED_CLIENT_BOX_HPP
#define NEXUS_MARKET_DATA_FEED_CLIENT_BOX_HPP
#include <memory>
#include <string>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
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

      void Publish(const MarketOrderImbalance& orderImbalance);

      void Publish(const SecurityBboQuote& bboQuote);

      void Publish(const SecurityMarketQuote& marketQuote);

      void Publish(const SecurityBookQuote& bookQuote);

      void Publish(const SecurityTimeAndSale& timeAndSale);

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

      void Close();

    private:
      struct VirtualMarketDataFeedClient {
        virtual ~VirtualMarketDataFeedClient() = default;
        virtual void Add(const SecurityInfo& securityInfo) = 0;
        virtual void Publish(const MarketOrderImbalance& orderImbalance) = 0;
        virtual void Publish(const SecurityBboQuote& bboQuote) = 0;
        virtual void Publish(const SecurityMarketQuote& marketQuote) = 0;
        virtual void Publish(const SecurityBookQuote& bookQuote) = 0;
        virtual void Publish(const SecurityTimeAndSale& timeAndSale) = 0;
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
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedMarketDataFeedClient final : VirtualMarketDataFeedClient {
        using MarketDataFeedClient = C;
        Beam::GetOptionalLocalPtr<MarketDataFeedClient> m_client;

        template<typename... Args>
        WrappedMarketDataFeedClient(Args&&... args);
        void Add(const SecurityInfo& securityInfo) override;
        void Publish(const MarketOrderImbalance& orderImbalance) override;
        void Publish(const SecurityBboQuote& bboQuote) override;
        void Publish(const SecurityMarketQuote& marketQuote) override;
        void Publish(const SecurityBookQuote& bookQuote) override;
        void Publish(const SecurityTimeAndSale& timeAndSale) override;
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

  inline void MarketDataFeedClientBox::Publish(
      const MarketOrderImbalance& orderImbalance) {
    return m_client->Publish(orderImbalance);
  }

  inline void MarketDataFeedClientBox::Publish(
      const SecurityBboQuote& bboQuote) {
    return m_client->Publish(bboQuote);
  }

  inline void MarketDataFeedClientBox::Publish(
      const SecurityMarketQuote& marketQuote) {
    return m_client->Publish(marketQuote);
  }

  inline void MarketDataFeedClientBox::Publish(
      const SecurityBookQuote& bookQuote) {
    return m_client->Publish(bookQuote);
  }

  inline void MarketDataFeedClientBox::Publish(
      const SecurityTimeAndSale& timeAndSale) {
    return m_client->Publish(timeAndSale);
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
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Publish(
      const MarketOrderImbalance& orderImbalance) {
    return m_client->Publish(orderImbalance);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Publish(
      const SecurityBboQuote& bboQuote) {
    return m_client->Publish(bboQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Publish(
      const SecurityMarketQuote& marketQuote) {
    return m_client->Publish(marketQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Publish(
      const SecurityBookQuote& bookQuote) {
    return m_client->Publish(bookQuote);
  }

  template<typename C>
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Publish(
      const SecurityTimeAndSale& timeAndSale) {
    return m_client->Publish(timeAndSale);
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
  void MarketDataFeedClientBox::WrappedMarketDataFeedClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
