#ifndef NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Sql/Conversions.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus {
  inline const auto& get_account_row() {
    static auto ROW = Viper::Row<Beam::DirectoryEntry>().
      add_column("account", &Beam::DirectoryEntry::m_id);
    return ROW;
  }

  inline const auto& get_order_info_row() {
    static auto ROW = Viper::Row<OrderInfo>().
      add_column("account",
        [] (auto& row) -> auto& {
          return row.m_fields.m_account.m_id;
        }).
      add_column("order_id", &OrderInfo::m_id).
      add_column("submission_account",
        [] (auto& row) {
          return row.m_submission_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_submission_account.m_id = column;
          row.m_submission_account.m_type =
            Beam::DirectoryEntry::Type::ACCOUNT;
        }).
      extend(Viper::Row<OrderFields>().
        add_column("symbol", Viper::varchar(16),
          [] (auto& row) {
            return row.m_security.get_symbol();
          },
          [] (auto& row, auto column) {
            row.m_security =
              Security(std::move(column), row.m_security.get_venue());
          }).
        add_column("venue", Viper::varchar(16),
          [] (auto& row) {
            return row.m_security.get_venue();
          },
          [] (auto& row, auto column) {
            row.m_security = Security(row.m_security.get_symbol(), column);
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
            return row.m_time_in_force.get_type();
          },
          [] (auto& row, auto column) {
            row.m_time_in_force =
              TimeInForce(column, row.m_time_in_force.get_expiry());
          }).
        add_column("time_in_force_expiry",
          [] (auto& row) {
            return Beam::to_sql_timestamp(row.m_time_in_force.get_expiry());
          },
          [] (auto& row, auto column) {
            row.m_time_in_force = TimeInForce(
              row.m_time_in_force.get_type(), Beam::from_sql_timestamp(column));
          }).
        add_column("additional_fields",
          [] (auto& row) {
            auto buffer = Beam::SharedBuffer();
            if(row.m_additional_fields.empty()) {
              return buffer;
            }
            auto sender = Beam::BinarySender<Beam::SharedBuffer>();
            sender.set(Beam::Ref(buffer));
            try {
              sender.shuttle(row.m_additional_fields);
            } catch(const Beam::SerializationException&) {
              boost::throw_with_location(OrderExecutionDataStoreException(
                "Unable to store additional fields."));
            }
            return buffer;
          },
          [] (auto& row, const auto& column) {
            if(!is_empty(column)) {
              auto receiver = Beam::BinaryReceiver<Beam::SharedBuffer>();
              receiver.set(Beam::Ref(column));
              try {
                receiver.shuttle(row.m_additional_fields);
              } catch(const Beam::SerializationException&) {
                boost::throw_with_location(OrderExecutionDataStoreException(
                  "Unable to load additional fields."));
              }
            }
          }), &OrderInfo::m_fields).
        add_column("shorting_flag", &OrderInfo::m_shorting_flag).
        set_primary_key("order_id");
    return ROW;
  }

  inline const auto& get_execution_report_row() {
    static auto ROW = Viper::Row<ExecutionReport>().
      add_column("order_id", &ExecutionReport::m_id).
      add_column("sequence", &ExecutionReport::m_sequence).
      add_column("status", &ExecutionReport::m_status).
      add_column("last_quantity", &ExecutionReport::m_last_quantity).
      add_column("last_price", &ExecutionReport::m_last_price).
      add_column("liquidity_flag", Viper::varchar(8),
        &ExecutionReport::m_liquidity_flag).
      add_column(
        "last_market", Viper::varchar(16), &ExecutionReport::m_last_market).
      add_column("execution_fee", &ExecutionReport::m_execution_fee).
      add_column("processing_fee", &ExecutionReport::m_processing_fee).
      add_column("commission", &ExecutionReport::m_commission).
      add_column("text", Viper::varchar(256), &ExecutionReport::m_text).
      add_index("order_id", "order_id");
    return ROW;
  }

  inline auto has_live_check(const Beam::Expression& expression) {
    struct IsLiveVisitor final : TraversalExpressionVisitor {
      bool m_has_check = false;

      void visit(const Beam::MemberAccessExpression& expression) override {
        if(expression.get_name() == "is_live" &&
            expression.get_expression().get_type() == typeid(OrderInfo)) {
          m_has_check = true;
        } else {
          TraversalExpressionVisitor::visit(expression);
        }
      }
    };
    auto visitor = IsLiveVisitor();
    expression.apply(visitor);
    return visitor.m_has_check;
  }
}

#endif
