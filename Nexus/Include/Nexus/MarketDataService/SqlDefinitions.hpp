#ifndef NEXUS_MARKET_DATA_SQL_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_SQL_DEFINITIONS_HPP
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Returns a row representing a venue. */
  inline const auto& get_venue_row() {
    static auto ROW = Viper::Row<Venue>().
      add_column("venue", Viper::varchar(16));
    return ROW;
  }

  /** Returns a row representing an order imbalance. */
  inline const auto& get_order_imbalance_row() {
    static auto ROW = Viper::Row<OrderImbalance>().
      extend(Viper::Row<Security>().
        add_column("symbol", Viper::varchar(16),
          [] (const auto& row) {
            return row.get_symbol();
          },
          [] (auto& row, auto value) {
            row = Security(std::move(value), row.get_venue());
          }).
        add_column("symbol_venue", Viper::varchar(16),
          [] (const auto& row) {
            return row.get_venue();
          },
          [] (auto& row, auto value) {
            row = Security(std::move(row.get_symbol()), value);
          }), &OrderImbalance::m_security).
      add_column("side", &OrderImbalance::m_side).
      add_column("size", &OrderImbalance::m_size).
      add_column("price", &OrderImbalance::m_reference_price);
    return ROW;
  }

  /** Returns a row representing a security. */
  inline const auto& get_security_row() {
    static auto ROW = Viper::Row<Security>().
      add_column("symbol", Viper::varchar(16),
        [] (const auto& row) {
          return row.get_symbol();
        },
        [] (auto& row, auto value) {
          row = Security(std::move(value), row.get_venue());
        }).
      add_column("venue", Viper::varchar(16),
        [] (const auto& row) {
          return row.get_venue();
        },
        [] (auto& row, auto value) {
          row = Security(std::move(row.get_symbol()), value);
        });
    return ROW;
  }

  /** Returns a row representing a SecurityInfo. */
  inline const auto& get_security_info_row() {
    static auto ROW = Viper::Row<SecurityInfo>().
      extend(Viper::Row<Security>().
        add_column("symbol", Viper::varchar(16),
          [] (const auto& row) {
            return row.get_symbol();
          },
          [] (auto& row, auto value) {
            row = Security(std::move(value), row.get_venue());
          }).
        add_column("venue", Viper::varchar(16),
          [] (const auto& row) {
            return row.get_venue();
          },
          [] (auto& row, auto value) {
            row = Security(std::move(row.get_symbol()), value);
          }), &SecurityInfo::m_security).
      add_column("name", Viper::varchar(256), &SecurityInfo::m_name).
      add_column("sector", Viper::varchar(256), &SecurityInfo::m_sector).
      add_column("board_lot", &SecurityInfo::m_board_lot).
      set_primary_key({"symbol", "venue"});
    return ROW;
  }

  /** Returns a row representing a bbo quote. */
  inline const auto& get_bbo_quote_row() {
    static auto ROW = Viper::Row<BboQuote>().
      extend(Viper::Row<Quote>().
        add_column("bid_price", &Quote::m_price).
        add_column("bid_size", &Quote::m_size), &BboQuote::m_bid).
      extend(Viper::Row<Quote>().
        add_column("ask_price", &Quote::m_price).
        add_column("ask_size", &Quote::m_size), &BboQuote::m_ask);
    return ROW;
  }

  /** Returns a row representing a book quote. */
  inline const auto& get_book_quote_row() {
    static auto ROW = Viper::Row<BookQuote>().
      add_column("mpid", Viper::varchar(16), &BookQuote::m_mpid).
      add_column("is_primary", &BookQuote::m_is_primary_mpid).
      add_column("quote_venue", Viper::varchar(16), &BookQuote::m_venue).
      extend(Viper::Row<Quote>().
        add_column("price", &Quote::m_price).
        add_column("size", &Quote::m_size).
        add_column("side", &Quote::m_side), &BookQuote::m_quote);
    return ROW;
  }

  /** Returns a row representing a time and sale. */
  inline const auto& get_time_and_sale_row() {
    static auto ROW = Viper::Row<TimeAndSale>().
      add_column("price", &TimeAndSale::m_price).
      add_column("size", &TimeAndSale::m_size).
      extend(Viper::Row<TimeAndSale::Condition>().
        add_column("condition_code", Viper::varchar(4),
          &TimeAndSale::Condition::m_code).
        add_column("condition_type", &TimeAndSale::Condition::m_type),
          &TimeAndSale::m_condition).
      add_column("market", Viper::varchar(16), &TimeAndSale::m_market_center).
      add_column("buyer_mpid", Viper::varchar(16), &TimeAndSale::m_buyer_mpid).
      add_column(
        "seller_mpid", Viper::varchar(16), &TimeAndSale::m_seller_mpid);
    return ROW;
  }
}

#endif
