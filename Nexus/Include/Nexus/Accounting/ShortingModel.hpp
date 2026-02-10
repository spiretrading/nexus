#ifndef NEXUS_SHORTING_MODEL_HPP
#define NEXUS_SHORTING_MODEL_HPP
#include <unordered_map>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Tracks whether an Order should be submitted as a short sale. */
  class ShortingModel {
    public:

      /**
       * Tracks a submission and returns whether it should be marked as a
       * short sale.
       * @param id The id to pull from the ExecutionReport when updating.
       * @param fields The OrderFields storing the details of the submission.
       * @return <code>true</code> iff the submission should be marked as a
       *         short sale.
       */
      bool submit(OrderId id, const OrderFields& fields);

      /**
       * Updates this model with the contents of an ExecutionReport.
       * @param report The ExecutionReport to update this model with.
       */
      void update(const ExecutionReport& report);

    private:
      struct PositionEntry {
        Quantity m_ask_quantity_pending;
        Quantity m_position;
      };
      struct OrderEntry {
        Ticker m_ticker;
        Side m_side;
        Quantity m_quantity;
        Quantity m_remaining_quantity;
      };
      std::unordered_map<OrderId, OrderEntry> m_order_entries;
      std::unordered_map<Ticker, PositionEntry> m_position_entries;

      PositionEntry& get_position(const Ticker& ticker);
  };

  inline bool ShortingModel::submit(OrderId id, const OrderFields& fields) {
    auto order_iterator = m_order_entries.find(id);
    if(order_iterator != m_order_entries.end()) {
      m_order_entries.erase(order_iterator);
    }
    auto order_entry = OrderEntry();
    order_entry.m_ticker = fields.m_ticker;
    order_entry.m_side = fields.m_side;
    order_entry.m_quantity = fields.m_quantity;
    order_entry.m_remaining_quantity = fields.m_quantity;
    m_order_entries.insert(std::pair(id, order_entry));
    auto& position = get_position(fields.m_ticker);
    if(fields.m_side == Side::ASK) {
      position.m_ask_quantity_pending += fields.m_quantity;
      return position.m_ask_quantity_pending > position.m_position;
    }
    return false;
  }

  inline void ShortingModel::update(const ExecutionReport& report) {
    auto order_iterator = m_order_entries.find(report.m_id);
    if(order_iterator == m_order_entries.end()) {
      return;
    }
    auto& order_entry = order_iterator->second;
    auto& position = get_position(order_entry.m_ticker);
    if(order_entry.m_side == Side::BID) {
      position.m_position += report.m_last_quantity;
    } else {
      position.m_position -= report.m_last_quantity;
      position.m_ask_quantity_pending -= report.m_last_quantity;
    }
    order_entry.m_remaining_quantity -=
      std::min(report.m_last_quantity, order_entry.m_remaining_quantity);
    if(order_entry.m_side == Side::ASK && is_terminal(report.m_status)) {
      position.m_ask_quantity_pending -= order_entry.m_remaining_quantity;
    }
    if(is_terminal(report.m_status)) {
      m_order_entries.erase(report.m_id);
    }
  }

  inline ShortingModel::PositionEntry&
      ShortingModel::get_position(const Ticker& ticker) {
    auto position_iterator = m_position_entries.find(ticker);
    if(position_iterator == m_position_entries.end()) {
      position_iterator =
        m_position_entries.insert(std::pair(ticker, PositionEntry())).first;
    }
    return position_iterator->second;
  }
}

#endif
