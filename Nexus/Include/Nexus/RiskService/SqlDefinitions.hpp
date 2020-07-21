#ifndef NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#include <cstdint>
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores a position and the account it belongs to. */
  struct PositionEntry {

    /** The account the position belongs to. */
    std::uint32_t m_account;

    /** The position stored. */
    RiskPortfolioPosition m_position;
  };

  /** Returns a row representing a PositionEntry. */
  inline const auto& GetPositionEntriesRow() {
    static auto ROW = Viper::Row<PositionEntry>().
      add_column("account", &PositionEntry::m_account).
      extend(Viper::Row<RiskPortfolioPosition>().
        extend(Viper::Row<RiskPortfolioPosition::Key>().
          extend(Viper::Row<Security>().
            add_column("symbol", Viper::varchar(16),
              [] (auto& row) {
                return row.GetSymbol();
              },
              [] (auto& row, auto column) {
                row = Security(std::move(column), row.GetMarket(),
                  row.GetCountry());
              }).
            add_column("market", Viper::varchar(16),
              [] (auto& row) {
                return row.GetMarket();
              },
              [] (auto& row, auto column) {
                row = Security(row.GetSymbol(), column, row.GetCountry());
              }).
            add_column("country",
              [] (auto& row) {
                return row.GetCountry();
              },
              [] (auto& row, auto column) {
                row = Security(row.GetSymbol(), row.GetMarket(), column);
              }),
            [] (auto& entry) -> decltype(auto) {
              return entry.m_index;
            }).
          add_column("currency",
            [] (auto& entry) -> decltype(auto) {
              return entry.m_currency;
            }),
          [] (auto& entry) -> decltype(auto) {
            return entry.m_key;
          }).
        add_column("quantity",
          [] (auto& entry) -> decltype(auto) {
            return entry.m_quantity;
          }).
        add_column("cost_basis",
          [] (auto& entry) -> decltype(auto) {
            return entry.m_costBasis;
          }), &PositionEntry::m_position).
      add_index("account", "account");
    return ROW;
  }

  /** Converts a RiskPortfolioPosition into a PositionEntry. */
  inline auto ConvertPositionSnapshotPositions(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return [=] (const RiskPortfolioPosition& position) {
      return PositionEntry{account.m_id, position};
    };
  }

  /** Stores a sequence and the account it belongs to. */
  struct PositionSequence {

    /** The account the sequence belongs to. */
    std::uint32_t m_account;

    /** The sequence stored. */
    Beam::Queries::Sequence m_sequence;
  };

  /** Returns a row representing a SequenceEntry. */
  inline const auto& GetPositionSequencesRow() {
    static auto ROW = Viper::Row<PositionSequence>().
      add_column("account", &PositionSequence::m_account).
      add_column("sequence", &PositionSequence::m_sequence).
      add_index("account", "account");
    return ROW;
  }

  /** Stores an excluded order id and the account it belongs to. */
  struct PositionExcludedOrderId {

    /** The account the order belongs to. */
    std::uint32_t m_account;

    /** The excluded order id. */
    OrderExecutionService::OrderId m_id;
  };

  /** Returns a row representing a PositionExcludedOrderId. */
  inline const auto& GetPositionExcludedOrdersRow() {
    static auto ROW = Viper::Row<PositionExcludedOrderId>().
      add_column("account", &PositionExcludedOrderId::m_account).
      add_column("id", &PositionExcludedOrderId::m_id).
      add_index("account", "account");
    return ROW;
  }
}

#endif
