#ifndef NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus::OrderExecutionService {
  inline const auto& GetAccountRow() {
    static auto ROW = Viper::Row<Beam::ServiceLocator::DirectoryEntry>().
      add_column("account", &Beam::ServiceLocator::DirectoryEntry::m_id);
    return ROW;
  }

  inline const auto& GetOrderInfoRow() {
    static auto ROW = Viper::Row<OrderInfo>().
      add_column("account",
        [] (auto& row) -> auto& {
          return row.m_fields.m_account.m_id;
        }).
      add_column("order_id", &OrderInfo::m_orderId).
      add_column("submission_account",
        [] (auto& row) {
          return row.m_submissionAccount.m_id;
        },
        [] (auto& row, auto column) {
          row.m_submissionAccount.m_id = column;
          row.m_submissionAccount.m_type =
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
        }).
      extend(Viper::Row<OrderFields>().
        add_column("symbol", Viper::varchar(16),
          [] (auto& row) {
            return row.m_security.GetSymbol();
          },
          [] (auto& row, auto column) {
            row.m_security = Security(std::move(column),
              row.m_security.GetMarket(), row.m_security.GetCountry());
          }).
        add_column("market", Viper::varchar(16),
          [] (auto& row) {
            return row.m_security.GetMarket();
          },
          [] (auto& row, auto column) {
            row.m_security = Security(row.m_security.GetSymbol(), column,
              row.m_security.GetCountry());
          }).
        add_column("country",
          [] (auto& row) {
            return row.m_security.GetCountry();
          },
          [] (auto& row, auto column) {
            row.m_security = Security(row.m_security.GetSymbol(),
              row.m_security.GetMarket(), column);
          }).
        add_column("currency", &OrderFields::m_currency).
        add_column("type", &OrderFields::m_type).
        add_column("side", &OrderFields::m_side).
        add_column("destination", Viper::varchar(16),
          &OrderFields::m_destination).
        add_column("quantity", &OrderFields::m_quantity).
        add_column("price", &OrderFields::m_price).
        add_column("time_in_force",
          [] (auto& row) {
            return row.m_timeInForce.GetType();
          },
          [] (auto& row, auto column) {
            row.m_timeInForce = TimeInForce(column,
              row.m_timeInForce.GetExpiry());
          }).
        add_column("time_in_force_expiry",
          [] (auto& row) {
            return Beam::ToSqlTimestamp(row.m_timeInForce.GetExpiry());
          },
          [] (auto& row, auto column) {
            row.m_timeInForce = TimeInForce(row.m_timeInForce.GetType(),
              Beam::FromSqlTimestamp(column));
          }).
        add_column("additional_fields",
          [] (auto& row) {
            auto buffer = Beam::IO::SharedBuffer();
            if(row.m_additionalFields.empty()) {
              return buffer;
            }
            auto sender = Beam::Serialization::BinarySender<
              Beam::IO::SharedBuffer>();
            sender.SetSink(Beam::Ref(buffer));
            try {
              sender.Shuttle(row.m_additionalFields);
            } catch(const Beam::Serialization::SerializationException&) {
              BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
                "Unable to store additional fields."));
            }
            return buffer;
          },
          [] (auto& row, const auto& column) {
            if(!column.IsEmpty()) {
              auto receiver = Beam::Serialization::BinaryReceiver<
                Beam::IO::SharedBuffer>();
              receiver.SetSource(Beam::Ref(column));
              try {
                receiver.Shuttle(row.m_additionalFields);
              } catch(const Beam::Serialization::SerializationException&) {
                BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
                  "Unable to load additional fields."));
              }
            }
          }), &OrderInfo::m_fields).
        add_column("shorting_flag", &OrderInfo::m_shortingFlag).
        set_primary_key("order_id");
    return ROW;
  }

  inline const auto& GetExecutionReportRow() {
    static auto ROW = Viper::Row<ExecutionReport>().
      add_column("order_id", &ExecutionReport::m_id).
      add_column("sequence", &ExecutionReport::m_sequence).
      add_column("status", &ExecutionReport::m_status).
      add_column("last_quantity", &ExecutionReport::m_lastQuantity).
      add_column("last_price", &ExecutionReport::m_lastPrice).
      add_column("liquidity_flag", Viper::varchar(8),
        &ExecutionReport::m_liquidityFlag).
      add_column("last_market", Viper::varchar(16),
        &ExecutionReport::m_lastMarket).
      add_column("execution_fee", &ExecutionReport::m_executionFee).
      add_column("processing_fee", &ExecutionReport::m_processingFee).
      add_column("commission", &ExecutionReport::m_commission).
      add_column("text", Viper::varchar(256), &ExecutionReport::m_text).
      add_index("order_id", "order_id");
    return ROW;
  }

  inline auto HasLiveCheck(const Beam::Queries::Expression& expression) {
    struct IsLiveVisitor final : Queries::TraversalExpressionVisitor {
      bool m_hasCheck = false;

      void Visit(const Beam::Queries::MemberAccessExpression& expression)
          override {
        if(expression.GetName() == "is_live" &&
            expression.GetExpression()->GetType() == Queries::OrderInfoType()) {
          m_hasCheck = true;
        } else {
          Queries::TraversalExpressionVisitor::Visit(expression);
        }
      }
    };
    auto visitor = IsLiveVisitor();
    expression->Apply(visitor);
    return visitor.m_hasCheck;
  }
}

#endif
