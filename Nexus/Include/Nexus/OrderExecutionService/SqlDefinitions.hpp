#ifndef NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_SQL_DEFINITIONS_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Viper/Row.hpp>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus::OrderExecutionService {
  inline const auto& GetAccountRow() {
    static auto ROW = Viper::Row<Beam::ServiceLocator::DirectoryEntry>().
      add_column("account",
      [] (auto& row) -> auto& {
        return row.m_id;
      });
    return ROW;
  }

  inline const auto& GetOrderInfoRow() {
    static auto ROW = Viper::Row<OrderInfo>().
      add_column("order_id", &OrderInfo::m_orderId).
      add_column("submission_account",
        [] (auto& row) -> auto& {
          return row.m_submissionAccount.m_id;
        },
        [] (auto& row, auto column) {
          row.m_submissionAccount.m_id = column;
          row.m_submissionAccount.m_type =
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
        }).
      add_column("symbol", Viper::varchar(16),
        [] (auto& row) -> auto& {
          return row.m_fields.m_security.GetSymbol();
        },
        [] (auto& row, auto column) {
          row.m_fields.m_security = Security(std::move(column),
            row.m_fields.m_security.GetMarket(),
            row.m_fields.m_security.GetCountry());
        }).
      add_column("market", Viper::varchar(16),
        [] (auto& row) {
          return std::string{row.m_fields.m_security.GetMarket().GetData()};
        },
        [] (auto& row, auto column) {
          row.m_fields.m_security = Security(
            row.m_fields.m_security.GetSymbol(), column,
            row.m_fields.m_security.GetCountry());
        }).
      add_column("country",
        [] (auto& row) {
          return static_cast<std::uint32_t>(
            row.m_fields.m_security.GetCountry());
        },
        [] (auto& row, auto column) {
          row.m_fields.m_security = Security(
            row.m_fields.m_security.GetSymbol(),
            row.m_fields.m_security.GetMarket(), CountryCode(column));
        }).
      add_column("currency",
        [] (auto& row) {
          return static_cast<std::uint32_t>(row.m_fields.m_currency.m_value);
        },
        [] (auto& row, auto column) {
          row.m_fields.m_currency = CurrencyId(static_cast<int>(column));
        }).
      add_column("type",
        [] (auto& row) {
          return static_cast<std::uint32_t>(row.m_fields.m_type);
        },
        [] (auto& row, auto column) {
          row.m_fields.m_type = OrderType(column);
        }).
      add_column("side",
        [] (auto& row) {
          return static_cast<std::uint32_t>(row.m_fields.m_side);
        },
        [] (auto& row, auto column) {
          row.m_fields.m_side = Side(column);
        }).
      add_column("destination", Viper::varchar(16),
        [] (auto& row) -> auto& {
          return row.m_fields.m_destination;
        }).
      add_column("quantity",
        [] (auto& row) {
          return row.m_fields.m_quantity.GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_fields.m_quantity = Quantity::FromRepresentation(column);
        }).
      add_column("price",
        [] (auto& row) {
          return static_cast<Quantity>(
            row.m_fields.m_price).GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_fields.m_price = Money(Quantity::FromRepresentation(column));
        }).
      add_column("time_in_force",
        [] (auto& row) {
          return static_cast<std::uint32_t>(
            row.m_fields.m_timeInForce.GetType());
        },
        [] (auto& row, auto column) {
          row.m_fields.m_timeInForce = TimeInForce(TimeInForce::Type(column),
            row.m_fields.m_timeInForce.GetExpiry());
        }).
      add_column("time_in_force_expiry",
        [] (auto& row) {
          return Beam::ToSqlTimestamp(row.m_fields.m_timeInForce.GetExpiry());
        },
        [] (auto& row, auto column) {
          row.m_fields.m_timeInForce = TimeInForce(
            row.m_fields.m_timeInForce.GetType(),
            Beam::FromSqlTimestamp(column));
        }).
      add_column("additional_fields",
        [] (auto& row) {
          if(row.m_fields.m_additionalFields.empty()) {
            return std::vector<std::byte>();
          }
          auto buffer = Beam::IO::SharedBuffer();
          auto sender = Beam::Serialization::BinarySender<
            Beam::IO::SharedBuffer>();
          sender.SetSink(Beam::Ref(buffer));
          try {
            sender.Shuttle(row.m_fields.m_additionalFields);
          } catch(const Beam::Serialization::SerializationException&) {
            BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
              "Unable to store additional fields."));
          }
          auto value = std::vector<std::byte>(buffer.GetSize());
          std::memcpy(value.data(), buffer.GetData(), buffer.GetSize());
          return value;
        },
        [] (auto& row, const auto& column) {
          if(!column.empty()) {
            auto receiver =
              Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer>();
            auto buffer = Beam::IO::SharedBuffer(column.data(), column.size());
            receiver.SetSource(Beam::Ref(buffer));
            try {
              receiver.Shuttle(row.m_fields.m_additionalFields);
            } catch(const Beam::Serialization::SerializationException&) {
              BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
                "Unable to load additional fields."));
            }
          }
        }).
      add_column("shorting_flag", &OrderInfo::m_shortingFlag).
      set_primary_key("order_id");
    return ROW;
  }

  inline const auto& GetExecutionReportRow() {
    static auto ROW = Viper::Row<ExecutionReport>().
      add_column("order_id", &ExecutionReport::m_id).
      add_column("status",
        [] (auto& row) {
          return static_cast<std::uint32_t>(row.m_status);
        },
        [] (auto& row, auto column) {
          row.m_status = OrderStatus(column);
        }).
      add_column("last_quantity",
        [] (auto& row) {
          return row.m_lastQuantity.GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_lastQuantity = Quantity::FromRepresentation(column);
        }).
      add_column("last_price",
        [] (auto& row) {
          return static_cast<Quantity>(row.m_lastPrice).GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_lastPrice = Money(Quantity::FromRepresentation(column));
        }).
      add_column("liquidity_flag", Viper::varchar(8),
        &ExecutionReport::m_liquidityFlag).
      add_column("last_market", Viper::varchar(16),
        &ExecutionReport::m_lastMarket).
      add_column("execution_fee",
        [] (auto& row) {
          return static_cast<Quantity>(row.m_executionFee).GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_executionFee = Money(Quantity::FromRepresentation(column));
        }).
      add_column("processing_fee",
        [] (auto& row) {
          return static_cast<Quantity>(row.m_processingFee).GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_processingFee = Money(Quantity::FromRepresentation(column));
        }).
      add_column("commission",
        [] (auto& row) {
          return static_cast<Quantity>(row.m_commission).GetRepresentation();
        },
        [] (auto& row, auto column) {
          row.m_commission = Money(Quantity::FromRepresentation(column));
        }).
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
    IsLiveVisitor visitor;
    expression->Apply(visitor);
    return visitor.m_hasCheck;
  }
}

#endif
