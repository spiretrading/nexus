#include <doctest/doctest.h>
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  const auto AUCTION_MAX_QUANTITY = Quantity(10000);

  auto MakeFeeTable() {
    auto feeTable = TsxFeeTable();
    PopulateFeeTable(Store(feeTable.m_continuousFeeTable));
    PopulateFeeTable(Store(feeTable.m_auctionFeeTable));
    feeTable.m_auctionFeeTable[static_cast<int>(
      TsxFeeTable::AuctionType::OPEN)][static_cast<int>(
        TsxFeeTable::AuctionIndex::MAX_CHARGE)] = AUCTION_MAX_QUANTITY *
        LookupAuctionFee(feeTable, TsxFeeTable::AuctionIndex::FEE,
          TsxFeeTable::AuctionType::OPEN);
    feeTable.m_auctionFeeTable[static_cast<int>(
      TsxFeeTable::AuctionType::CLOSE)][static_cast<int>(
        TsxFeeTable::AuctionIndex::MAX_CHARGE)] = AUCTION_MAX_QUANTITY *
        LookupAuctionFee(feeTable, TsxFeeTable::AuctionIndex::FEE,
          TsxFeeTable::AuctionType::CLOSE);
    PopulateFeeTable(Store(feeTable.m_oddLotFeeList));
    return feeTable;
  }

  auto MakeOrderFields(Money price) {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      price);
  }

  auto MakeHiddenOrderFields(Money price) {
    auto fields = MakeOrderFields(price);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }
}

TEST_SUITE("TsxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_continuousFeeTable,
      LookupContinuousFee, TsxFeeTable::PRICE_CLASS_COUNT,
      TsxFeeTable::TYPE_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_auctionFeeTable, LookupAuctionFee,
      TsxFeeTable::AUCTION_INDEX_COUNT, TsxFeeTable::AUCTION_TYPE_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = Money::ZERO;
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = 0;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1,
        TsxFeeTable::Classification::DEFAULT, std::placeholders::_2,
        std::placeholders::_3), expectedFee);
  }

  TEST_CASE("active_interlisted_subdollar") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupContinuousFee(feeTable,
      TsxFeeTable::PriceClass::SUBDOLLAR, TsxFeeTable::Type::ACTIVE);
    auto fields = MakeOrderFields(50 * Money::CENT);
    fields.m_quantity = 100;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(CalculateFee, std::placeholders::_1,
        TsxFeeTable::Classification::INTERLISTED, std::placeholders::_2,
        std::placeholders::_3), expectedFee);
  }

  TEST_CASE("passive_interlisted_subdollar") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupContinuousFee(feeTable,
      TsxFeeTable::PriceClass::SUBDOLLAR, TsxFeeTable::Type::PASSIVE);
    auto fields = MakeOrderFields(50 * Money::CENT);
    fields.m_quantity = 100;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(CalculateFee, std::placeholders::_1,
        TsxFeeTable::Classification::INTERLISTED, std::placeholders::_2,
        std::placeholders::_3), expectedFee);
  }

  TEST_CASE("opening_auction") {
    auto feeTable = MakeFeeTable();
    auto perShareFee = LookupAuctionFee(feeTable,
      TsxFeeTable::AuctionIndex::FEE, TsxFeeTable::AuctionType::OPEN);
    auto maxFee = LookupAuctionFee(feeTable,
      TsxFeeTable::AuctionIndex::MAX_CHARGE, TsxFeeTable::AuctionType::OPEN);
    {
      auto quantity = Quantity(100);
      auto fields = MakeOrderFields(5 * Money::ONE);
      fields.m_quantity = quantity;
      TestPerShareFeeCalculation(feeTable, fields, "O",
        std::bind(CalculateFee, std::placeholders::_1,
          TsxFeeTable::Classification::DEFAULT, std::placeholders::_2,
          std::placeholders::_3), perShareFee);
    }
    {
      auto quantity = AUCTION_MAX_QUANTITY + 100;
      auto fields = MakeOrderFields(Money::CENT);
      fields.m_quantity = quantity;
      TestFeeCalculation(feeTable, fields, "O",
        std::bind(CalculateFee, std::placeholders::_1,
          TsxFeeTable::Classification::DEFAULT, std::placeholders::_2,
          std::placeholders::_3), maxFee);
    }
  }
}
