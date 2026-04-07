#ifndef NEXUS_FEE_TABLE_TEST_UTILITIES_HPP
#define NEXUS_FEE_TABLE_TEST_UTILITIES_HPP
#include <array>
#include <concepts>
#include <cstdlib>
#include <Beam/Pointers/Out.hpp>
#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Tests {

  /**
   * Populates a fee table with incremental CENT values.
   * @param table The fee table to populate.
   */
  template<std::size_t COLUMNS>
  void populate_fee_table(Beam::Out<std::array<Money, COLUMNS>> table) {
    for(auto i = 0; i < COLUMNS; ++i) {
      (*table)[i] = ((std::rand() % 100) + 1) * Money::CENT;
    }
  }

  /**
   * Populates a fee table with incremental CENT values.
   * @param table The fee table to populate.
   */
  template<std::size_t ROWS, std::size_t COLUMNS>
  void populate_fee_table(
      Beam::Out<std::array<std::array<Money, COLUMNS>, ROWS>> table) {
    for(auto i = 0; i < ROWS; ++i) {
      for(auto j = 0; j < COLUMNS; ++j) {
        (*table)[i][j] = ((std::rand() % 100) + 1) * Money::CENT;
      }
    }
  }

  /**
   * Tests indexing into a fee table.
   * @param parent The parent table containing all the fees.
   * @param table The specific fee table within the <i>parent</i> to test.
   * @param index_function The function used to index the <i>parent</i>.
   * @param expected_rows The expected number of rows.
   * @param expected_columns The expected number of columns.
   */
  template<typename FeeTable, std::size_t ROWS, std::size_t COLUMNS,
    typename IndexFunction>
  void test_fee_table_index(const FeeTable& parent,
      const std::array<std::array<Money, COLUMNS>, ROWS>& table,
      const IndexFunction& index_function, std::size_t expected_columns,
      std::size_t expected_rows) {
    using ColumnType =
      typename boost::function_traits<IndexFunction>::arg2_type;
    using RowType = typename boost::function_traits<IndexFunction>::arg3_type;
    REQUIRE(COLUMNS == expected_columns);
    REQUIRE(ROWS == expected_rows);
    for(auto i = 0; i < ROWS; ++i) {
      for(auto j = 0; j < COLUMNS; ++j) {
        auto row = static_cast<RowType>(i);
        auto column = static_cast<ColumnType>(j);
        auto fee = index_function(parent, column, row);
        auto expected_fee = table[i][j];
        REQUIRE(expected_fee == fee);
      }
    }
  }

  /**
   * Tests a fee calculation.
   * @param table The fee table to test.
   * @param price The price of the trade to test.
   * @param quantity The trade's quantity.
   * @param flag The trade's LiquidityFlag.
   * @param expected_fee The expected fee.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_fee_calculation(const FeeTable& table, Money price,
      Quantity quantity, LiquidityFlag flag, Money expected_fee,
      CalculateFeeType&& calculate_fee) requires std::invocable<
        CalculateFeeType, const FeeTable&, const ExecutionReport&> &&
          std::same_as<std::invoke_result_t<
            CalculateFeeType, const FeeTable&, const ExecutionReport&>, Money> {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = boost::lexical_cast<std::string>(flag);
    auto calculated_total =
      std::forward<CalculateFeeType>(calculate_fee)(table, report);
    REQUIRE(calculated_total == expected_fee);
  }

  /**
   * Tests a fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields that were submitted.
   * @param flag The trade's liquidity flag.
   * @param expected_fee The expected fee.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_fee_calculation(const FeeTable& table, const OrderFields& fields,
      const std::string& flag, Money expected_fee,
      CalculateFeeType&& calculate_fee) requires std::invocable<
        CalculateFeeType, const FeeTable&, const ExecutionReport&> &&
          std::same_as<std::invoke_result_t<
            CalculateFeeType, const FeeTable&, const ExecutionReport&>, Money> {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = flag;
    auto calculated_total =
      std::forward<CalculateFeeType>(calculate_fee)(table, report);
    REQUIRE(calculated_total == expected_fee);
  }

  /**
   * Tests a fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields that were submitted.
   * @param flag The trade's liquidity flag.
   * @param calculateFee The function used to calculate the fee.
   * @param expectedFee The expected fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_fee_calculation(const FeeTable& table, const OrderFields& fields,
      std::string flag, Money expected_fee,
      CalculateFeeType&& calculate_fee) requires
        std::invocable<CalculateFeeType, const FeeTable&, const OrderFields&,
          const ExecutionReport&> &&
            std::same_as<std::invoke_result_t<CalculateFeeType, const FeeTable&,
              const OrderFields&, const ExecutionReport&>, Money> {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = std::move(flag);
    auto calculated_total = calculate_fee(table, fields, report);
    REQUIRE(calculated_total == expected_fee);
  }

  /**
   * Tests a notional value fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields used to produce an ExecutionReport.
   * @param expected_rate The expected rate to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_notional_value_fee_calculation(const FeeTable& table,
      const OrderFields& fields, boost::rational<int> expected_rate,
      CalculateFeeType&& calculate_fee) {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    auto calculated_total =
      std::forward<CalculateFeeType>(calculate_fee)(table, report);
    auto expected_total =
      expected_rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_total == expected_total);
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields used to produce an ExecutionReport.
   * @param flag The trade's LiquidityFlag.
   * @param quantity The last quantity filled.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(const FeeTable& table,
      const OrderFields& fields, const std::string& flag,
      Quantity quantity, Money expected_fee, CalculateFeeType&& calculate_fee) {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = fields.m_price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = flag;
    auto calculated_total =
      std::forward<CalculateFeeType>(calculate_fee)(table, fields, report);
    auto expected_total = report.m_last_quantity * expected_fee;
    REQUIRE(calculated_total == expected_total);
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields used to produce an ExecutionReport.
   * @param flag The trade's LiquidityFlag.
   * @param quantity The last quantity filled.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(
      const FeeTable& table, const OrderFields& fields, LiquidityFlag flag,
      Quantity quantity, Money expected_fee, CalculateFeeType&& calculate_fee) {
    test_per_share_fee_calculation(
      table, fields, boost::lexical_cast<std::string>(flag), quantity,
      expected_fee, std::forward<CalculateFeeType>(calculate_fee));
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields used to produce an ExecutionReport.
   * @param flag The trade's LiquidityFlag.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(
      const FeeTable& table, const OrderFields& fields, const std::string& flag,
      Money expected_fee, CalculateFeeType&& calculate_fee) {
    test_per_share_fee_calculation(table, fields, flag, fields.m_quantity,
      expected_fee, std::forward<CalculateFeeType>(calculate_fee));
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param fields The OrderFields used to produce an ExecutionReport.
   * @param flag The trade's LiquidityFlag.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(
      const FeeTable& table, const OrderFields& fields, LiquidityFlag flag,
      Money expected_fee, CalculateFeeType&& calculate_fee) {
    test_per_share_fee_calculation(
      table, fields, boost::lexical_cast<std::string>(flag), expected_fee,
      std::forward<CalculateFeeType>(calculate_fee));
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param price The price of the trade to test.
   * @param quantity The trade's quantity.
   * @param flag The trade's LiquidityFlag.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(const FeeTable& table, Money price,
      Quantity quantity, const std::string& flag, Money expected_fee,
      CalculateFeeType&& calculate_fee) {
    auto report =
      ExecutionReport(0, boost::posix_time::second_clock::universal_time());
    report.m_last_price = price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = flag;
    auto calculated_total =
      std::forward<CalculateFeeType>(calculate_fee)(table, report);
    auto expected_total = report.m_last_quantity * expected_fee;
    REQUIRE(calculated_total == expected_total);
  }

  /**
   * Tests a per share fee calculation.
   * @param table The fee table to test.
   * @param price The price of the trade to test.
   * @param quantity The trade's quantity.
   * @param flag The trade's LiquidityFlag.
   * @param expected_fee The expected fee to use in the calculation.
   * @param calculate_fee The function used to calculate the fee.
   */
  template<typename FeeTable, typename CalculateFeeType>
  void test_per_share_fee_calculation(const FeeTable& table, Money price,
      Quantity quantity, LiquidityFlag flag, Money expected_fee,
      CalculateFeeType&& calculate_fee) {
    test_per_share_fee_calculation(table, price, quantity,
      boost::lexical_cast<std::string>(flag), expected_fee,
      std::forward<CalculateFeeType>(calculate_fee));
  }
}

#endif
