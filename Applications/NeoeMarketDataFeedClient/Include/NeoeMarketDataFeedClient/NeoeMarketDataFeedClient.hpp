#ifndef NEXUS_NEOE_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_NEOE_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/Stamp/StampMessage.hpp"
#include "NeoeMarketDataFeedClient/NeoeConfiguration.hpp"
#include "NeoeMarketDataFeedClient/NeoeServiceAccessClient.hpp"

namespace Nexus {

  /**
   * Parses packets from the NEOE data feed.
   * @param <M> The type of MarketDataFeedClient used to update the
   *        MarketDataServer.
   * @param <S> The type of service access client receiving messages.
   * @param <T> The type of TimeClient used for timestamps.
   */
  template<typename M, typename S, typename T>
  class NeoeMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of channel receiving the market data feed. */
      using ServiceAccessClient = Beam::dereference_t<S>;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a NeoeMarketDataFeedClient.
       * @param config The configuration to use.
       * @param feed_client Initializes the MarketDataFeedClient.
       * @param service_access_client The service access client receiving
       *        messages.
       * @param time_client The TimeClient used for timestamps.
       */
      template<typename MF, typename SF, typename TF>
      NeoeMarketDataFeedClient(NeoeConfiguration config, MF&& feed_client,
        SF&& service_access_client, TF&& time_client);

      ~NeoeMarketDataFeedClient();

      void close();

    private:
      NeoeConfiguration m_config;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<S> m_service_access_client;
      Beam::local_ptr_t<T> m_time_client;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      NeoeMarketDataFeedClient(const NeoeMarketDataFeedClient&) = delete;
      NeoeMarketDataFeedClient& operator =(
        const NeoeMarketDataFeedClient&) = delete;
      static Quantity get_board_lot_portion(Quantity quantity, Money price);
      static Quantity round_to_board_lot_portion(
        Quantity quantity, Money price);
      boost::optional<boost::posix_time::ptime> get_timestamp(
        const StampMessage& message, int index);
      const std::string& get_mpid(const std::string& broker_number) const;
      std::string get_order_id(const boost::optional<std::string>& symbol,
        const boost::optional<std::string>& broker_number,
        const std::string& order_number);
      void handle_quote(const StampMessage& message);
      void handle_last_sale_trade_report(const StampMessage& message);
      void handle_order_info(const StampMessage& message);
      void handle_booked_order(const StampMessage& message);
      void handle_cancelled_order(const StampMessage& message);
      void handle_price_assigned_order(const StampMessage& message);
      void handle_order_or_cancel_confirmation_report(
        const StampMessage& message);
      void handle_order_trader_report(const StampMessage& message);
      void read_loop();
  };

  template<typename M, typename S, typename T>
  template<typename MF, typename SF, typename TF>
  NeoeMarketDataFeedClient<M, S, T>::NeoeMarketDataFeedClient(
      NeoeConfiguration config, MF&& feed_client,
      SF&& service_access_client, TF&& time_client)
      try : m_config(std::move(config)),
            m_feed_client(std::forward<MF>(feed_client)),
            m_service_access_client(std::forward<SF>(service_access_client)),
            m_time_client(std::forward<TF>(time_client)),
            m_read_loop(Beam::spawn(
              std::bind(&NeoeMarketDataFeedClient::read_loop, this))) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the NEOE market data feed client."));
  }

  template<typename M, typename S, typename T>
  NeoeMarketDataFeedClient<M, S, T>::~NeoeMarketDataFeedClient() {
    close();
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_client->close();
    m_service_access_client->close();
    m_feed_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename S, typename T>
  Quantity NeoeMarketDataFeedClient<M, S, T>::get_board_lot_portion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity - (quantity % 500);
    } else {
      return quantity - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  Quantity NeoeMarketDataFeedClient<M, S, T>::round_to_board_lot_portion(
      Quantity quantity, Money price) {
    if(price < 10 * Money::CENT) {
      return quantity + 1000 - (quantity % 1000);
    } else if(price < 99 * Money::CENT) {
      return quantity + 500 - (quantity % 500);
    } else {
      return quantity + 100 - (quantity % 100);
    }
  }

  template<typename M, typename S, typename T>
  boost::optional<boost::posix_time::ptime> NeoeMarketDataFeedClient<M, S, T>::
      get_timestamp(const StampMessage& message, int index) {
    auto timestamp =
      message.get_business_field<boost::posix_time::ptime>(index);
    if(timestamp) {
      *timestamp += m_config.m_time_offset;
    }
    return timestamp;
  }

  template<typename M, typename S, typename T>
  const std::string& NeoeMarketDataFeedClient<M, S, T>::get_mpid(
      const std::string& broker_number) const {
    auto normalized_broker_number = [&] () -> std::string {
      auto i = broker_number.find_first_not_of('0');
      if(i == std::string::npos) {
        return "0";
      }
      return broker_number.substr(i);
    }();
    auto i = m_config.m_mpid_mappings.find(normalized_broker_number);
    if(i != m_config.m_mpid_mappings.end()) {
      return i->second;
    }
    return broker_number;
  }

  template<typename M, typename S, typename T>
  std::string NeoeMarketDataFeedClient<M, S, T>::get_order_id(
      const boost::optional<std::string>& symbol,
      const boost::optional<std::string>& broker_number,
      const std::string& order_number) {
    auto result = std::string();
    if(symbol) {
      result = *symbol;
      result += '-';
    }
    if(broker_number) {
      if(broker_number->size() == 1) {
        result += '0';
        result += '0';
      } else if(broker_number->size() == 2) {
        result += '0';
      }
      result.append(*broker_number);
      result += '-';
    }
    result += order_number;
    return result;
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_quote(
      const StampMessage& message) {
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bid_price = message.get_business_field<Money>(196, 0);
    if(!bid_price || *bid_price == Money::ZERO) {
      return;
    }
    auto bid_volume = message.get_business_field<Quantity>(64, 0);
    if(!bid_volume) {
      return;
    }
    auto ask_price = message.get_business_field<Money>(196, 1);
    if(!ask_price || *ask_price == Money::ZERO) {
      return;
    }
    auto ask_volume = message.get_business_field<Quantity>(64, 1);
    if(!ask_volume) {
      return;
    }
    auto security = Security(std::move(*symbol), DefaultVenues::NEOE);
    auto bid = make_bid(*bid_price, *bid_volume);
    auto ask = make_ask(*ask_price, *ask_volume);
    auto bbo = BboQuote(bid, ask, m_time_client->get_time());
    m_feed_client->publish(SecurityBboQuote(bbo, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_last_sale_trade_report(
      const StampMessage& message) {
    auto business_action = message.get_business_field<std::string>(5);
    if(!business_action) {
      return;
    }
    if(*business_action == "Cancelled") {
      return;
    }
    auto timestamp = get_timestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto price = message.get_business_field<Money>(41);
    if(!price) {
      return;
    }
    auto volume = message.get_business_field<Quantity>(64);
    if(!volume) {
      return;
    }
    auto exchange_id = message.get_business_field<std::string>(247);
    if(!exchange_id) {
      return;
    }
    auto buyer_mpid =
      get_mpid(message.get_business_field<std::string>(70, 0).value_or(""));
    auto seller_mpid =
      get_mpid(message.get_business_field<std::string>(70, 1).value_or(""));
    auto security = Security(std::move(*symbol), DefaultVenues::NEOE);
    auto condition = TimeAndSale::Condition();
    condition.m_code = "@";
    auto time_and_sale = TimeAndSale(*timestamp, *price, *volume,
      std::move(condition), *exchange_id, std::move(buyer_mpid),
      std::move(seller_mpid));
    m_feed_client->publish(
      SecurityTimeAndSale(time_and_sale, std::move(security)));
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_order_info(
      const StampMessage& message) {
    handle_booked_order(message);
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_booked_order(
      const StampMessage& message) {
    auto non_resident_flag = message.get_business_field<std::string>(168);
    if(non_resident_flag && *non_resident_flag == "Y") {
      return;
    }
    auto settlement_terms = message.get_business_field<std::string>(53);
    if(settlement_terms) {
      return;
    }
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto order_number = message.get_business_field<std::string>(40);
    if(!order_number) {
      return;
    }
    auto mpid_field = message.get_business_field<std::string>(70);
    if(!mpid_field || mpid_field->empty()) {
      return;
    }
    auto mpid = *mpid_field;
    auto mpid_name_iterator = m_config.m_mpid_mappings.find(mpid);
    if(mpid_name_iterator != m_config.m_mpid_mappings.end()) {
      mpid = mpid_name_iterator->second;
    }
    auto price = message.get_business_field<Money>(196);
    if(!price) {
      return;
    }
    auto quantity = message.get_business_field<Quantity>(64);
    if(!quantity) {
      return;
    }
    auto side = message.get_business_field<Side>(5);
    if(!side) {
      return;
    }
    auto timestamp = get_timestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto broker_number = message.get_business_field<std::string>(70);
    auto order_id = get_order_id(symbol, broker_number, *order_number);
    auto security = Security(std::move(*symbol), DefaultVenues::NEOE);
    *quantity = get_board_lot_portion(*quantity, *price);
    m_feed_client->add_order(security, DefaultVenues::NEOE, mpid, false,
      order_id, *side, *price, *quantity, *timestamp);
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_cancelled_order(
      const StampMessage& message) {
    auto order_number = message.get_business_field<std::string>(40);
    if(!order_number) {
      return;
    }
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto broker_number = message.get_business_field<std::string>(70);
    auto timestamp = get_timestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto order_id = get_order_id(symbol, broker_number, *order_number);
    m_feed_client->remove_order(order_id, *timestamp);
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_price_assigned_order(
      const StampMessage& message) {
    auto price = message.get_business_field<Money>(196);
    if(!price) {
      return;
    }
    auto order_number = message.get_business_field<std::string>(40);
    if(!order_number) {
      return;
    }
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto broker_number = message.get_business_field<std::string>(70);
    auto timestamp = get_timestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto order_id = get_order_id(symbol, broker_number, *order_number);
    m_feed_client->modify_order_price(order_id, *price, *timestamp);
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_order_or_cancel_confirmation_report(
      const StampMessage& message) {
    auto confirmation_type = message.get_business_field<std::string>(16);
    if(!confirmation_type) {
      return;
    }
    if(*confirmation_type == "Booked") {
      handle_booked_order(message);
    } else if(*confirmation_type == "Cancelled") {
      handle_cancelled_order(message);
    } else if(*confirmation_type == "PriceAssigned") {
      handle_price_assigned_order(message);
    }
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::handle_order_trader_report(
      const StampMessage& message) {
    auto timestamp = get_timestamp(message, 57);
    if(!timestamp) {
      return;
    }
    auto volume = message.get_business_field<Quantity>(64);
    if(!volume) {
      return;
    }
    auto price = message.get_business_field<Money>(41);
    if(!price) {
      return;
    }
    auto symbol = message.get_business_field<std::string>(55);
    if(!symbol ||
        m_config.m_securities.find(*symbol) == m_config.m_securities.end()) {
      return;
    }
    auto bid_broker_number = message.get_business_field<std::string>(70, 0);
    auto bid_order_number = message.get_business_field<std::string>(40, 0);
    if(bid_order_number) {
      auto bid_order_id =
        get_order_id(symbol, bid_broker_number, *bid_order_number);
      auto display_volume = message.get_business_field<Quantity>(150, 0);
      if(display_volume) {
        *display_volume = get_board_lot_portion(*display_volume, *price);
        m_feed_client->modify_order_size(
          bid_order_id, *display_volume, *timestamp);
      } else {
        *volume = round_to_board_lot_portion(*volume, *price);
        m_feed_client->offset_order_size(bid_order_id, -*volume, *timestamp);
      }
    }
    auto ask_broker_number = message.get_business_field<std::string>(70, 1);
    auto ask_order_number = message.get_business_field<std::string>(40, 1);
    if(ask_order_number) {
      auto ask_order_id =
        get_order_id(symbol, ask_broker_number, *ask_order_number);
      auto display_volume = message.get_business_field<Quantity>(150, 1);
      if(display_volume) {
        *display_volume = get_board_lot_portion(*display_volume, *price);
        m_feed_client->modify_order_size(
          ask_order_id, *display_volume, *timestamp);
      } else {
        *volume = round_to_board_lot_portion(*volume, *price);
        m_feed_client->offset_order_size(ask_order_id, -*volume, *timestamp);
      }
    }
  }

  template<typename M, typename S, typename T>
  void NeoeMarketDataFeedClient<M, S, T>::read_loop() {
    static constexpr auto BUSINESS_CLASS_FIELD_ID = 6;
    while(true) {
      auto message = std::optional<StampMessage>();
      try {
        message.emplace(m_service_access_client->read());
      } catch(const Beam::EndOfFileException&) {
        break;
      }
      if(m_config.m_is_logging_messages) {
        std::cout << message->get_header().m_sequence_number << ": " <<
          std::string(message->get_business_content_data(),
            message->get_business_content_size()) << "\n";
      }
      auto business_class =
        message->get_business_field<std::string>(BUSINESS_CLASS_FIELD_ID);
      if(!business_class) {
        continue;
      }
      if(*business_class == "OrderCancelResp") {
        handle_order_or_cancel_confirmation_report(*message);
      } else if(*business_class == "Quote") {
        handle_quote(*message);
      } else if(*business_class == "TradeReport") {
        if(m_config.m_is_time_and_sale_feed) {
          handle_last_sale_trade_report(*message);
        } else {
          handle_order_trader_report(*message);
        }
      } else if(*business_class == "OrderInfo") {
        handle_order_info(*message);
      }
    }
  }
}

#endif
