#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = CseFeeTable();
    populate_fee_table(out(table.m_fee_table));
    populate_fee_table(out(table.m_interlisted_fee_table));
    populate_fee_table(out(table.m_etf_fee_table));
    populate_fee_table(out(table.m_cse_listed_fee_table));
    populate_fee_table(out(table.m_cse_open_fee_table));
    populate_fee_table(out(table.m_cse_close_fee_table));
    return table;
  }

  std::string make_cse_liquidity_flag(LiquidityFlag flag) {
    if(flag == LiquidityFlag::ACTIVE) {
      return "T";
    }
    return "P";
  }

  Money make_price(CseFeeTable::PriceClass price_class) {
    if(price_class == CseFeeTable::PriceClass::SUBDOLLAR) {
      return 99 * Money::CENT;
    }
    return Money::ONE;
  }
}

TEST_SUITE("CseFeeHandling") {
  TEST_CASE("fee_table_indexing") {
    auto table = make_fee_table();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            table.m_fee_table[t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(
            get_default_fee(table, type, price_class, liquidity) == expected);
        }
      }
    }
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = table.m_interlisted_fee_table[
            t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(get_interlisted_fee(table, type, price_class, liquidity) ==
            expected);
        }
      }
    }
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            table.m_etf_fee_table[t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(get_etf_fee(table, type, price_class, liquidity) == expected);
        }
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = table.m_cse_listed_fee_table[p][l];
        REQUIRE(get_cse_listed_fee(table, price_class, liquidity) == expected);
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = table.m_cse_open_fee_table[p][l];
        REQUIRE(get_open_fee(table, price_class, liquidity) == expected);
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = table.m_cse_close_fee_table[p][l];
        REQUIRE(get_close_fee(table, price_class, liquidity) == expected);
      }
    }
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto report = ExecutionReport(0, second_clock::universal_time());
    report.m_last_price = Money::ONE;
    report.m_last_quantity = 0;
    report.m_liquidity_flag = "T";
    auto fee =
      calculate_fee(table, CseFeeTable::CseListing::CSE_LISTED, report);
    REQUIRE(fee == Money::ZERO);
  }

  TEST_CASE("cse_listed_fee") {
    auto table = make_fee_table();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = get_cse_listed_fee(table, price_class, liquidity);
        auto price = make_price(price_class);
        test_per_share_fee_calculation(table, price, 100, liquidity, expected,
          [] (const auto& table, const auto& report) {
            return calculate_fee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          });
      }
    }
  }

  TEST_CASE("interlisted_fee") {
    auto table = make_fee_table();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            get_interlisted_fee(table, type, price_class, liquidity);
          auto flag = make_cse_liquidity_flag(liquidity);
          if(type == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = make_price(price_class);
          test_per_share_fee_calculation(table, price, 100, flag, expected,
            [] (const auto& table, const auto& report) {
              return calculate_fee(
                table, CseFeeTable::CseListing::INTERLISTED, report);
            });
        }
      }
    }
  }

  TEST_CASE("etf_fee") {
    auto table = make_fee_table();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = get_etf_fee(table, type, price_class, liquidity);
          auto flag = make_cse_liquidity_flag(liquidity);
          if(type == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = make_price(price_class);
          test_per_share_fee_calculation(table, price, 100, flag, expected,
            [] (const auto& table, const auto& report) {
              return calculate_fee(table, CseFeeTable::CseListing::ETF, report);
            });
        }
      }
    }
  }

  TEST_CASE("default_fee") {
    auto table = make_fee_table();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto type = static_cast<CseFeeTable::TradeType>(t);
          auto price_class = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = get_default_fee(table, type, price_class, liquidity);
          auto flag = make_cse_liquidity_flag(liquidity);
          if(type == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = make_price(price_class);
          test_per_share_fee_calculation(table, price, 100, flag, expected,
            [] (const auto& table, const auto& report) {
              return calculate_fee(
                table, CseFeeTable::CseListing::DEFAULT, report);
            });
        }
      }
    }
  }

  TEST_CASE("open_fee") {
    auto table = make_fee_table();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = get_open_fee(table, price_class, liquidity);
        auto flag = make_cse_liquidity_flag(liquidity);
        flag += "CLO";
        auto price = make_price(price_class);
        test_per_share_fee_calculation(table, price, 100, flag, expected,
          [] (const auto& table, const auto& report) {
            return calculate_fee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          });
      }
    }
  }

  TEST_CASE("close_fee") {
    auto table = make_fee_table();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto price_class = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = get_close_fee(table, price_class, liquidity);
        auto flag = make_cse_liquidity_flag(liquidity);
        flag += "CLM";
        auto price = make_price(price_class);
        test_per_share_fee_calculation(table, price, 100, flag, expected,
          [] (const auto& table, const auto& report) {
            return calculate_fee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          });
      }
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    auto price = Money::ONE;
    auto quantity = 100;
    auto expected = get_default_fee(table, CseFeeTable::TradeType::DEFAULT,
      CseFeeTable::PriceClass::DEFAULT, LiquidityFlag::ACTIVE);
    test_per_share_fee_calculation(table, price, quantity, "?", expected,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, CseFeeTable::CseListing::DEFAULT, report);
      });
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto price = Money::ONE;
    auto quantity = 100;
    auto expected = get_default_fee(table, CseFeeTable::TradeType::DEFAULT,
      CseFeeTable::PriceClass::DEFAULT, LiquidityFlag::ACTIVE);
    test_per_share_fee_calculation(table, price, quantity, "", expected,
      [](const auto& table, const auto& report) {
        return calculate_fee(table, CseFeeTable::CseListing::DEFAULT, report);
      });
  }
}
