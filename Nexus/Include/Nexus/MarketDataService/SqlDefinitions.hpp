#ifndef NEXUS_MARKET_DATA_SQL_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_SQL_DEFINITIONS_HPP
#include <Viper/Row.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  //! Returns a row representing a market code.
  inline const auto& GetMarketCodeRow() {
    static auto ROW = Viper::Row<MarketCode>().
      add_column("market", Viper::varchar(16),
        [] (const auto& row) {
          return std::string{row.GetData()};
        },
        [] (auto& row, const auto& value) {
          row = value;
        });
    return ROW;
  }

  //! Returns a row representing an order imbalance.
  inline const auto& GetOrderImbalanceRow() {
    static auto ROW = Viper::Row<OrderImbalance>().
      add_column("symbol", Viper::varchar(16),
        [] (const auto& row) -> auto& {
          return row.m_security.GetSymbol();
        },
        [] (auto& row, auto value) {
          row.m_security = Security(std::move(value),
            row.m_security.GetMarket(), row.m_security.GetCountry());
        }).
      add_column("symbol_market", Viper::varchar(16),
        [] (const auto& row) {
          return std::string{row.m_security.GetMarket().GetData()};
        },
        [] (auto& row, auto value) {
          row.m_security = Security(std::move(row.m_security.GetSymbol()),
            value, row.m_security.GetCountry());
        }).
      add_column("country",
        [] (const auto& row) {
          return std::uint32_t(row.m_security.GetCountry());
        },
        [] (auto& row, auto value) {
          row.m_security = Security(std::move(row.m_security.GetSymbol()),
            row.m_security.GetMarket(), CountryCode(value));
        }).
      add_column("side",
        [] (const auto& row) {
          return static_cast<std::uint32_t>(row.m_side);
        },
        [] (auto& row, auto value) {
          row.m_side = static_cast<Side>(value);
        }).
      add_column("size",
        [] (const auto& row) {
          return row.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_size = Quantity::FromRepresentation(value);
        }).
      add_column("price",
        [] (const auto& row) {
          return static_cast<Quantity>(
            row.m_referencePrice).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_referencePrice = Money(Quantity::FromRepresentation(value));
        });
    return ROW;
  }

  //! Returns a row representing a security.
  inline const auto& GetSecurityRow() {
    static auto ROW = Viper::Row<Security>().
      add_column("symbol", Viper::varchar(16),
        [] (const auto& row) -> auto& {
          return row.GetSymbol();
        },
        [] (auto& row, auto value) {
          row = Security(std::move(value), row.GetMarket(), row.GetCountry());
        }).
      add_column("country",
        [] (const auto& row) {
          return std::uint32_t(row.GetCountry());
        },
        [] (auto& row, auto value) {
          row = Security(std::move(row.GetSymbol()), row.GetMarket(),
            CountryCode(value));
        });
    return ROW;
  }

  //! Returns a row representing a bbo quote.
  inline const auto& GetBboQuoteRow() {
    static auto ROW = Viper::Row<BboQuote>().
      add_column("bid_price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_bid.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_bid.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("bid_size",
        [] (const auto& row) {
          return row.m_bid.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_bid.m_size = Quantity::FromRepresentation(value);
        }).
      add_column("ask_price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_ask.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_ask.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("ask_size",
        [] (const auto& row) {
          return row.m_ask.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_ask.m_size = Quantity::FromRepresentation(value);
        });
    return ROW;
  }

  //! Returns a row representing a market quote.
  inline const auto& GetMarketQuoteRow() {
    static auto ROW = Viper::Row<MarketQuote>().
      add_column("market", Viper::varchar(16),
        [] (const auto& row) {
          return std::string{row.m_market.GetData()};
        },
        [] (auto& row, auto& value) {
          row.m_market = value;
        }).
      add_column("bid_price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_bid.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_bid.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("bid_size",
        [] (const auto& row) {
          return row.m_bid.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_bid.m_size = Quantity::FromRepresentation(value);
        }).
      add_column("ask_price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_ask.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_ask.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("ask_size",
        [] (const auto& row) {
          return row.m_ask.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_ask.m_size = Quantity::FromRepresentation(value);
        });
    return ROW;
  }

  //! Returns a row representing a book quote.
  inline const auto& GetBookQuoteRow() {
    static auto ROW = Viper::Row<BookQuote>().
      add_column("mpid", Viper::varchar(16), &BookQuote::m_mpid).
      add_column("is_primary", &BookQuote::m_isPrimaryMpid).
      add_column("market", Viper::varchar(16),
        [] (const auto& row) {
          return std::string{row.m_market.GetData()};
        },
        [] (auto& row, auto& value) {
          row.m_market = value;
        }).
      add_column("price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_quote.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_quote.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("size",
        [] (const auto& row) {
          return row.m_quote.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_quote.m_size = Quantity::FromRepresentation(value);
        }).
      add_column("side",
        [] (const auto& row) {
          return static_cast<std::uint32_t>(row.m_quote.m_side);
        },
        [] (auto& row, auto value) {
          row.m_quote.m_side = static_cast<Side>(value);
        });
    return ROW;
  }

  //! Returns a row representing a time and sale.
  inline const auto& GetTimeAndSaleRow() {
    static auto ROW = Viper::Row<TimeAndSale>().
      add_column("price",
        [] (const auto& row) {
          return static_cast<Quantity>(row.m_price).GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_price = Money(Quantity::FromRepresentation(value));
        }).
      add_column("size",
        [] (const auto& row) {
          return row.m_size.GetRepresentation();
        },
        [] (auto& row, auto value) {
          row.m_size = Quantity::FromRepresentation(value);
        }).
      add_column("condition_code", Viper::varchar(4),
        [] (const auto& row) -> auto& {
          return row.m_condition.m_code;
        }).
      add_column("condition_type",
        [] (const auto& row) -> auto& {
          return row.m_condition.m_type;
        }).
      add_column("market", Viper::varchar(16), &TimeAndSale::m_marketCenter);
    return ROW;
  }
}

#endif
