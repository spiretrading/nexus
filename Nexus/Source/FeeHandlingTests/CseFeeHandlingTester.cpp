#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto MakeFeeTable() {
    auto feeTable = CseFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    PopulateFeeTable(Store(feeTable.m_interlistedFeeTable));
    PopulateFeeTable(Store(feeTable.m_etfFeeTable));
    PopulateFeeTable(Store(feeTable.m_cseListedFeeTable));
    PopulateFeeTable(Store(feeTable.m_cseOpenFeeTable));
    PopulateFeeTable(Store(feeTable.m_cseCloseFeeTable));
    return feeTable;
  }

  std::string MakeCseLiquidityFlag(LiquidityFlag flag) {
    if(flag == LiquidityFlag::ACTIVE) {
      return "T";
    }
    return "P";
  }

  Money MakePrice(CseFeeTable::PriceClass priceClass) {
    if(priceClass == CseFeeTable::PriceClass::SUBDOLLAR) {
      return 99 * Money::CENT;
    }
    return Money::ONE;
  }
}

TEST_SUITE("CseFeeHandling") {
  TEST_CASE("fee_table_indexing") {
    auto feeTable = MakeFeeTable();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            feeTable.m_feeTable[t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(GetDefaultFee(feeTable, tradeType, priceClass, liquidity) ==
            expected);
        }
      }
    }
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = feeTable.m_interlistedFeeTable[
            t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(GetInterlistedFee(
            feeTable, tradeType, priceClass, liquidity) == expected);
        }
      }
    }
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            feeTable.m_etfFeeTable[t * CseFeeTable::PRICE_CLASS_COUNT + p][l];
          REQUIRE(
            GetEtfFee(feeTable, tradeType, priceClass, liquidity) == expected);
        }
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = feeTable.m_cseListedFeeTable[p][l];
        REQUIRE(GetCseListedFee(feeTable, priceClass, liquidity) == expected);
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = feeTable.m_cseOpenFeeTable[p][l];
        REQUIRE(GetOpenFee(feeTable, priceClass, liquidity) == expected);
      }
    }
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = feeTable.m_cseCloseFeeTable[p][l];
        REQUIRE(GetCloseFee(feeTable, priceClass, liquidity) == expected);
      }
    }
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto executionReport = ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 0;
    executionReport.m_liquidityFlag = "T";
    auto fee = CalculateFee(
      feeTable, CseFeeTable::CseListing::CSE_LISTED, executionReport);
    REQUIRE(fee == Money::ZERO);
  }

  TEST_CASE("cse_listed_fee") {
    auto feeTable = MakeFeeTable();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = GetCseListedFee(feeTable, priceClass, liquidity);
        auto price = MakePrice(priceClass);
        TestPerShareFeeCalculation(feeTable, price, 100, liquidity,
          [] (const auto& table, const auto& report) {
            return CalculateFee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          }, expected);
      }
    }
  }

  TEST_CASE("interlisted_fee") {
    auto feeTable = MakeFeeTable();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected =
            GetInterlistedFee(feeTable, tradeType, priceClass, liquidity);
          auto flag = MakeCseLiquidityFlag(liquidity);
          if(tradeType == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = MakePrice(priceClass);
          TestPerShareFeeCalculation(feeTable, price, 100, flag,
            [] (const auto& table, const auto& report) {
              return CalculateFee(
                table, CseFeeTable::CseListing::INTERLISTED, report);
            }, expected);
        }
      }
    }
  }

  TEST_CASE("etf_fee") {
    auto feeTable = MakeFeeTable();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = GetEtfFee(feeTable, tradeType, priceClass, liquidity);
          auto flag = MakeCseLiquidityFlag(liquidity);
          if(tradeType == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = MakePrice(priceClass);
          TestPerShareFeeCalculation(feeTable, price, 100, flag,
            [] (const auto& table, const auto& report) {
              return CalculateFee(table, CseFeeTable::CseListing::ETF, report);
            }, expected);
        }
      }
    }
  }

  TEST_CASE("default_fee") {
    auto feeTable = MakeFeeTable();
    for(auto t = 0; t < CseFeeTable::TRADE_TYPE_COUNT; ++t) {
      for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
        for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
          auto tradeType = static_cast<CseFeeTable::TradeType>(t);
          auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
          auto liquidity = static_cast<LiquidityFlag>(l);
          auto expected = GetDefaultFee(feeTable, tradeType, priceClass, liquidity);
          auto flag = MakeCseLiquidityFlag(liquidity);
          if(tradeType == CseFeeTable::TradeType::DARK) {
            flag += "CD";
          }
          auto price = MakePrice(priceClass);
          TestPerShareFeeCalculation(feeTable, price, 100, flag,
            [] (const auto& table, const auto& report) {
              return CalculateFee(
                table, CseFeeTable::CseListing::DEFAULT, report);
            }, expected);
        }
      }
    }
  }

  TEST_CASE("open_fee") {
    auto feeTable = MakeFeeTable();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = GetOpenFee(feeTable, priceClass, liquidity);
        auto flag = MakeCseLiquidityFlag(liquidity);
        flag += "CLO";
        auto price = MakePrice(priceClass);
        TestPerShareFeeCalculation(feeTable, price, 100, flag,
          [] (const auto& table, const auto& report) {
            return CalculateFee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          }, expected);
      }
    }
  }

  TEST_CASE("close_fee") {
    auto feeTable = MakeFeeTable();
    for(auto p = 0; p < CseFeeTable::PRICE_CLASS_COUNT; ++p) {
      for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
        auto priceClass = static_cast<CseFeeTable::PriceClass>(p);
        auto liquidity = static_cast<LiquidityFlag>(l);
        auto expected = GetCloseFee(feeTable, priceClass, liquidity);
        auto flag = MakeCseLiquidityFlag(liquidity);
        flag += "CLM";
        auto price = MakePrice(priceClass);
        TestPerShareFeeCalculation(feeTable, price, 100, flag,
          [] (const auto& table, const auto& report) {
            return CalculateFee(
              table, CseFeeTable::CseListing::CSE_LISTED, report);
          }, expected);
      }
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto price = Money::ONE;
    auto quantity = 100;
    auto expected = GetDefaultFee(feeTable, CseFeeTable::TradeType::DEFAULT,
      CseFeeTable::PriceClass::DEFAULT, LiquidityFlag::ACTIVE);
    TestPerShareFeeCalculation(feeTable, price, quantity, "?",
      [] (const auto& table, const auto& report) {
        return CalculateFee(table, CseFeeTable::CseListing::DEFAULT, report);
      }, expected);
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto price = Money::ONE;
    auto quantity = 100;
    auto expected = GetDefaultFee(feeTable, CseFeeTable::TradeType::DEFAULT,
      CseFeeTable::PriceClass::DEFAULT, LiquidityFlag::ACTIVE);
    TestPerShareFeeCalculation(feeTable, price, quantity, "",
      [](const auto& table, const auto& report) {
        return CalculateFee(table, CseFeeTable::CseListing::DEFAULT, report);
      }, expected);
  }
}
