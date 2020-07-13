#ifndef NEXUS_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_MARKET_DATA_FEED_CLIENT_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/Authenticator.hpp>
#include <Beam/ServiceLocator/ServiceEntry.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/MarketDataService/MarketDataFeedServices.hpp"

namespace Nexus::MarketDataService {

  /** Client used to access the MarketDataFeedServlet.
      \tparam OrderIdType The type used to represent order ids.
      \tparam SamplingTimerType The type of Timer used to sample market data
              sent to the servlet.
      \tparam MessageProtocolType The type of MessageProtocol used to send and
              receive messages.
      \tparam HeartbeatTimerType The type of Timer used for heartbeats.
   */
  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  class MarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type used to represent order ids.
      using OrderId = OrderIdType;

      //! The type of Timer used for sampling.
      using SamplingTimer = Beam::GetTryDereferenceType<SamplingTimerType>;

      //! The type of MessageProtocol used to send and receive messages.
      using MessageProtocol = MessageProtocolType;

      //! The type of Timer used for heartbeats.
      using HeartbeatTimer = Beam::GetTryDereferenceType<HeartbeatTimerType>;

      //! The type of ServiceProtocol to use.
      using ServiceProtocolClient = Beam::Services::ServiceProtocolClient<
        MessageProtocolType, HeartbeatTimerType>;

      //! The type of Authenticator to use.
      using Authenticator = typename Beam::ServiceLocator::Authenticator<
        ServiceProtocolClient>::type;

      //! Constructs a MarketDataFeedClient.
      /*!
        \param channel Initializes the Channel to the ServiceProtocol server.
        \param authenticator The Authenticator to use.
        \param samplingTimer Initializes the SamplingTimer.
        \param heartbeatTimer Initializes the Timer used for heartbeats.
      */
      template<typename ChannelForward, typename SamplingTimerForward,
        typename HeartbeatTimerForward>
      MarketDataFeedClient(ChannelForward&& channel,
        const Authenticator& authenticator,
        SamplingTimerForward&& samplingTimer,
        HeartbeatTimerForward&& heartbeatTimer);

      ~MarketDataFeedClient();

      //! Adds or updates a SecurityInfo.
      /*!
        \param securityInfo The SecurityInfo to add or update.
      */
      void Add(const SecurityInfo& securityInfo);

      //! Publishes an OrderImbalance.
      /*!
        \param orderImbalance The OrderImbalance to publish.
      */
      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance);

      //! Publishes a BboQuote.
      /*!
        \param bboQuote The BboQuote to publish.
      */
      void PublishBboQuote(const SecurityBboQuote& bboQuote);

      //! Publishes a MarketQuote.
      /*!
        \param marketQuote The MarketQuote to publish.
      */
      void PublishMarketQuote(const SecurityMarketQuote& marketQuote);

      //! Sets a BookQuote.
      /*!
        \param bookQuote The BookQuote to set.
      */
      void SetBookQuote(const SecurityBookQuote& bookQuote);

      //! Adds an order.
      /*!
        \param security The Security the order belongs to.
        \param market The market the order belongs to.
        \param mpid The MPID submitting the order.
        \param isPrimaryMpid Whether the <i>mpid</i> is the <i>market</i>'s
               primary MPID.
        \param id The order id.
        \param side The order's Side.
        \param price The price of the order.
        \param size The size of the order.
        \param timestamp The Order's timestamp.
      */
      void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const OrderId& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp);

      //! Modifies the size of an order.
      /*!
        \param id The order id.
        \param size The order's new size.
        \param timestamp The modification's timestamp.
      */
      void ModifyOrderSize(const OrderId& id, Quantity size,
        const boost::posix_time::ptime& timestamp);

      //! Adds an offset to the size of an order.
      /*!
        \param id The order id.
        \param delta The change in the order's size.
        \param timestamp The modification's timestamp.
      */
      void OffsetOrderSize(const OrderId& id, Quantity delta,
        const boost::posix_time::ptime& timestamp);

      //! Modifies the price of an order.
      /*!
        \param id The order id.
        \param price The order's new price.
        \param timestamp The modification's timestamp.
      */
      void ModifyOrderPrice(const OrderId& id, Money price,
        const boost::posix_time::ptime& timestamp);

      //! Deletes an order.
      /*!
        \param id The order id.
        \param timestamp The modification's timestamp.
      */
      void DeleteOrder(const OrderId& id,
        const boost::posix_time::ptime& timestamp);

      //! Publishes a TimeAndSale.
      /*!
        \param timeAndSale The TimeAndSale to publish.
      */
      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale);

      void Open();

      void Close();

    private:
      struct OrderEntry {
        Security m_security;
        MarketCode m_market;
        std::string m_mpid;
        bool m_isPrimaryMpid;
        Side m_side;
        Money m_price;
        Quantity m_size;

        OrderEntry(const Security& security, MarketCode market,
          const std::string& mpid, bool isPrimaryMpid, Side side, Money price,
          Quantity size);
      };
      struct QuoteUpdates {
        boost::optional<SecurityBboQuote> m_bboQuote;
        std::unordered_map<MarketCode, SecurityMarketQuote> m_marketQuotes;
        std::vector<SecurityBookQuote> m_askBook;
        std::vector<SecurityBookQuote> m_bidBook;
        std::vector<SecurityTimeAndSale> m_timeAndSales;
      };
      mutable boost::mutex m_mutex;
      ServiceProtocolClient m_client;
      Authenticator m_authenticator;
      Beam::GetOptionalLocalPtr<SamplingTimerType> m_samplingTimer;
      std::unordered_map<Security, QuoteUpdates> m_quoteUpdates;
      std::vector<MarketOrderImbalance> m_orderImbalances;
      std::unordered_map<OrderId, OrderEntry> m_orders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_callbacks;

      void Shutdown();
      void UpdateBookSampling(const SecurityBookQuote& bookQuote);
      void LockedAddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const OrderId& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp);
      void LockedDeleteOrder(typename std::unordered_map<OrderId,
        OrderEntry>::iterator& orderIterator,
        const boost::posix_time::ptime& timestamp);
      void OnTimerExpired(const Beam::Threading::Timer::Result& result);
  };

  //! Finds a MarketDataFeedService for a specified country.
  /*!
    \param country The country to service.
    \param serviceLocatorClient The ServiceLocatorClient used to locate
           services.
    \return The ServiceEntry belonging to the MarketDataFeedService for the
            specified <i>country</i>.
  */
  template<typename ServiceLocatorClient>
  boost::optional<Beam::ServiceLocator::ServiceEntry>
      FindMarketDataFeedService(CountryCode country,
      ServiceLocatorClient& serviceLocatorClient) {
    auto services = serviceLocatorClient.Locate(FEED_SERVICE_NAME);
    for(auto& entry : services) {
      auto& properties = entry.GetProperties();
      auto countriesProperty = properties.Get("countries");
      if(!countriesProperty.is_initialized()) {
        return entry;
      } else if(auto countriesList = boost::get<std::vector<Beam::JsonValue>>(
          &*countriesProperty)) {
        for(auto countryEntry : *countriesList) {
          if(auto value = boost::get<double>(&countryEntry)) {
            if(static_cast<CountryCode>(static_cast<std::uint16_t>(*value)) ==
                country) {
              return entry;
            }
          }
        }
      }
    }
    return boost::none;
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::OrderEntry::OrderEntry(const Security& security,
      MarketCode market, const std::string& mpid, bool isPrimaryMpid, Side side,
      Money price, Quantity size)
      : m_security(security),
        m_market(market),
        m_mpid(mpid),
        m_isPrimaryMpid(isPrimaryMpid),
        m_side(side),
        m_price(price),
        m_size(size) {}

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  template<typename ChannelForward, typename SamplingTimerForward,
    typename HeartbeatTimerForward>
  MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::MarketDataFeedClient(ChannelForward&& channel,
      const Authenticator& authenticator, SamplingTimerForward&& samplingTimer,
      HeartbeatTimerForward&& heartbeatTimer)
      : m_client(std::forward<ChannelForward>(channel),
          std::forward<HeartbeatTimerForward>(heartbeatTimer)),
        m_authenticator(authenticator),
        m_samplingTimer(std::forward<SamplingTimerForward>(samplingTimer)) {
    RegisterMarketDataFeedMessages(Beam::Store(m_client.GetSlots()));
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::~MarketDataFeedClient() {
    Close();
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::Add(const SecurityInfo& securityInfo) {
    Beam::Services::SendRecordMessage<SetSecurityInfoMessage>(m_client,
      securityInfo);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::PublishBboQuote(const SecurityBboQuote& bboQuote) {
    auto lock = boost::lock_guard(m_mutex);
    auto& updates = m_quoteUpdates[bboQuote.GetIndex()];
    updates.m_bboQuote = bboQuote;
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote) {
    auto lock = boost::lock_guard(m_mutex);
    m_quoteUpdates[marketQuote.GetIndex()].m_marketQuotes[
      marketQuote->m_market] = marketQuote;
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::SetBookQuote(const SecurityBookQuote& bookQuote) {
    auto id = bookQuote.GetIndex().GetSymbol() + '-' +
      boost::lexical_cast<std::string>(bookQuote.GetIndex().GetCountry()) +
      bookQuote->m_mpid + '-' + bookQuote->m_quote.m_price.ToString() +
      ToChar(bookQuote->m_quote.m_side);
    auto lock = boost::lock_guard(m_mutex);
    auto orderIterator = m_orders.find(id);
    if(orderIterator == m_orders.end()) {
      LockedAddOrder(bookQuote.GetIndex(), bookQuote->m_market,
        bookQuote->m_mpid, bookQuote->m_isPrimaryMpid, id,
        bookQuote->m_quote.m_side, bookQuote->m_quote.m_price,
        bookQuote->m_quote.m_size, bookQuote->m_timestamp);
    } else {
      LockedDeleteOrder(orderIterator, bookQuote->m_timestamp);
      if(bookQuote->m_quote.m_size != 0) {
        LockedAddOrder(bookQuote.GetIndex(), bookQuote->m_market,
          bookQuote->m_mpid, bookQuote->m_isPrimaryMpid, id,
          bookQuote->m_quote.m_side, bookQuote->m_quote.m_price,
          bookQuote->m_quote.m_size, bookQuote->m_timestamp);
      }
    }
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::AddOrder(const Security& security, MarketCode market,
      const std::string& mpid, bool isPrimaryMpid, const OrderId& id, Side side,
      Money price, Quantity size, const boost::posix_time::ptime& timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    LockedAddOrder(security, market, mpid, isPrimaryMpid, id, side, price,
      size, timestamp);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::ModifyOrderSize(const OrderId& id, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto orderIterator = m_orders.find(id);
    if(orderIterator == m_orders.end()) {
      return;
    }
    auto entry = orderIterator->second;
    LockedDeleteOrder(orderIterator, timestamp);
    LockedAddOrder(entry.m_security, entry.m_market, entry.m_mpid,
      entry.m_isPrimaryMpid, id, entry.m_side, entry.m_price, size, timestamp);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::OffsetOrderSize(const OrderId& id, Quantity delta,
      const boost::posix_time::ptime& timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto orderIterator = m_orders.find(id);
    if(orderIterator == m_orders.end()) {
      return;
    }
    auto entry = orderIterator->second;
    LockedDeleteOrder(orderIterator, timestamp);
    LockedAddOrder(entry.m_security, entry.m_market, entry.m_mpid,
      entry.m_isPrimaryMpid, id, entry.m_side, entry.m_price,
      entry.m_size + delta, timestamp);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::ModifyOrderPrice(const OrderId& id, Money price,
      const boost::posix_time::ptime& timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto orderIterator = m_orders.find(id);
    if(orderIterator == m_orders.end()) {
      return;
    }
    auto entry = orderIterator->second;
    LockedDeleteOrder(orderIterator, timestamp);
    LockedAddOrder(entry.m_security, entry.m_market, entry.m_mpid,
      entry.m_isPrimaryMpid, id, entry.m_side, price, entry.m_size, timestamp);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::DeleteOrder(const OrderId& id,
      const boost::posix_time::ptime& timestamp) {
    auto lock = boost::lock_guard(m_mutex);
    auto orderIterator = m_orders.find(id);
    if(orderIterator == m_orders.end()) {
      return;
    }
    LockedDeleteOrder(orderIterator, timestamp);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale) {
    auto lock = boost::lock_guard(m_mutex);
    auto& updates = m_quoteUpdates[timeAndSale.GetIndex()];
    updates.m_timeAndSales.push_back(timeAndSale);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance) {
    auto lock = boost::lock_guard(m_mutex);
    m_orderImbalances.push_back(orderImbalance);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::ServiceLocator::OpenAndAuthenticate(m_authenticator, m_client);
      m_samplingTimer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Beam::Threading::Timer::Result>(
        std::bind(&MarketDataFeedClient::OnTimerExpired, this,
        std::placeholders::_1)));
      m_samplingTimer->Start();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::Shutdown() {
    m_client.Close();
    m_samplingTimer->Cancel();
    m_callbacks.Break();
    m_openState.SetClosed();
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::UpdateBookSampling(
      const SecurityBookQuote& bookQuote) {
    auto book = [&] {
      if(bookQuote->m_quote.m_side == Side::ASK) {
        return &(m_quoteUpdates[bookQuote.GetIndex()].m_askBook);
      } else {
        BEAM_ASSERT(bookQuote->m_quote.m_side == Side::BID);
        return &(m_quoteUpdates[bookQuote.GetIndex()].m_bidBook);
      }
    }();
    auto quoteIterator = std::lower_bound(book->begin(), book->end(), bookQuote,
      &BookQuoteListingComparator);
    if(quoteIterator == book->end()) {
      book->push_back(bookQuote);
    } else if((*quoteIterator)->m_quote.m_price == bookQuote->m_quote.m_price &&
        (*quoteIterator)->m_mpid == bookQuote->m_mpid) {
      (*quoteIterator)->m_quote.m_size += bookQuote->m_quote.m_size;
      (*quoteIterator)->m_timestamp = bookQuote->m_timestamp;
    } else {
      book->insert(quoteIterator, bookQuote);
    }
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::LockedAddOrder(const Security& security,
      MarketCode market, const std::string& mpid, bool isPrimaryMpid,
      const OrderId& id, Side side, Money price, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    if(size <= 0) {
      return;
    }
    auto orderIterator = m_orders.find(id);
    if(orderIterator != m_orders.end()) {
      LockedDeleteOrder(orderIterator, timestamp);
    }
    m_orders.insert(std::make_pair(id, OrderEntry(security, market, mpid,
      isPrimaryMpid, side, price, size)));
    auto bookQuote = BookQuote(mpid, isPrimaryMpid, market,
      Quote(price, size, side), timestamp);
    UpdateBookSampling(Beam::Queries::IndexedValue(
      std::move(bookQuote), security));
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::LockedDeleteOrder(
      typename std::unordered_map<OrderId, OrderEntry>::iterator& orderIterator,
      const boost::posix_time::ptime& timestamp) {
    auto& entry = orderIterator->second;
    auto bookQuote = BookQuote(entry.m_mpid, entry.m_isPrimaryMpid,
      entry.m_market, Quote(entry.m_price, -entry.m_size, entry.m_side),
      timestamp);
    UpdateBookSampling(Beam::Queries::IndexedValue(
      std::move(bookQuote), entry.m_security));
    m_orders.erase(orderIterator);
  }

  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
  void MarketDataFeedClient<OrderIdType, SamplingTimerType, MessageProtocolType,
      HeartbeatTimerType>::OnTimerExpired(
      const Beam::Threading::Timer::Result& result) {
    auto messages = std::vector<MarketDataFeedMessage>();
    auto quoteUpdates = std::unordered_map<Security, QuoteUpdates>();
    auto orderImbalances = std::vector<MarketOrderImbalance>();
    {
      auto lock = boost::lock_guard(m_mutex);
      quoteUpdates.swap(m_quoteUpdates);
      orderImbalances.swap(m_orderImbalances);
    }
    for(auto& [security, updates] : quoteUpdates) {
      if(updates.m_bboQuote.is_initialized()) {
        messages.push_back(std::move(*updates.m_bboQuote));
      }
      std::transform(updates.m_marketQuotes.begin(),
        updates.m_marketQuotes.end(), std::back_inserter(messages),
        [] (const auto& quote) -> decltype(auto) {
          return std::move(quote.second);
        });
      std::copy_if(std::make_move_iterator(updates.m_askBook.begin()),
        std::make_move_iterator(updates.m_askBook.end()),
        std::back_inserter(messages),
        [] (const auto& quote) {
          return quote->m_quote.m_size != 0;
        });
      std::copy_if(std::make_move_iterator(updates.m_bidBook.begin()),
        std::make_move_iterator(updates.m_bidBook.end()),
        std::back_inserter(messages),
        [] (const auto& quote) {
          return quote->m_quote.m_size != 0;
        });
      std::move(updates.m_timeAndSales.begin(), updates.m_timeAndSales.end(),
        std::back_inserter(messages));
    }
    std::move(orderImbalances.begin(), orderImbalances.end(),
      std::back_inserter(messages));
    if(!messages.empty()) {
      Beam::Services::SendRecordMessage<SendMarketDataFeedMessages>(m_client,
        messages);
    }
    m_samplingTimer->Start();
  }
}

#endif
