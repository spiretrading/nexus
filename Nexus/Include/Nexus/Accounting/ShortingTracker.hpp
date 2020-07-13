#ifndef NEXUS_SHORTING_TRACKER_HPP
#define NEXUS_SHORTING_TRACKER_HPP
#include <unordered_map>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Accounting {

  /** Tracks whether an Order should be submitted as a short sale. */
  class ShortingTracker {
    public:

      /**
       * Tracks a submission and returns whether it should be marked as a
       * short sale.
       * @param id The id to pull from the ExecutionReport when updating.
       * @param orderFields The OrderFields storing the details of the
       *        submission.
       * @return <code>true</code> iff the submission should be marked as a
       *         short sale.
       */
      bool Submit(OrderExecutionService::OrderId id,
        const OrderExecutionService::OrderFields& orderFields);

      /**
       * Updates this tracker with the contents of an ExecutionReport.
       * @param executionReport The ExecutionReport to update this tracker with.
       */
      void Update(const OrderExecutionService::ExecutionReport&
        executionReport);

    private:
      struct PositionEntry {
        Quantity m_askQuantityPending;
        Quantity m_position;
      };
      struct OrderEntry {
        Security m_security;
        Side m_side;
        Quantity m_quantity;
        Quantity m_remainingQuantity;

        OrderEntry();
      };
      std::unordered_map<OrderExecutionService::OrderId, OrderEntry>
        m_orderIdToOrderEntry;
      std::unordered_map<Security, PositionEntry> m_securityToPositionEntry;

      PositionEntry& GetPosition(const Security& security);
  };

  inline ShortingTracker::OrderEntry::OrderEntry()
    : m_side(Side::BID),
      m_quantity(0),
      m_remainingQuantity(0) {}

  inline bool ShortingTracker::Submit(OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& orderFields) {
    auto orderIterator = m_orderIdToOrderEntry.find(id);
    if(orderIterator != m_orderIdToOrderEntry.end()) {
      m_orderIdToOrderEntry.erase(orderIterator);
    }
    auto orderEntry = OrderEntry();
    orderEntry.m_security = orderFields.m_security;
    orderEntry.m_side = orderFields.m_side;
    orderEntry.m_quantity = orderFields.m_quantity;
    orderEntry.m_remainingQuantity = orderFields.m_quantity;
    m_orderIdToOrderEntry.insert(std::pair(id, orderEntry));
    auto& position = GetPosition(orderFields.m_security);
    if(orderFields.m_side == Side::ASK) {
      position.m_askQuantityPending += orderFields.m_quantity;
      return position.m_askQuantityPending > position.m_position;
    }
    return false;
  }

  inline void ShortingTracker::Update(
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto orderIterator = m_orderIdToOrderEntry.find(executionReport.m_id);
    if(orderIterator == m_orderIdToOrderEntry.end()) {
      return;
    }
    auto& orderEntry = orderIterator->second;
    auto& position = GetPosition(orderEntry.m_security);
    if(orderEntry.m_side == Side::BID) {
      position.m_position += executionReport.m_lastQuantity;
    } else {
      position.m_position -= executionReport.m_lastQuantity;
      position.m_askQuantityPending -= executionReport.m_lastQuantity;
    }
    orderEntry.m_remainingQuantity -= std::min(executionReport.m_lastQuantity,
      orderEntry.m_remainingQuantity);
    if(orderEntry.m_side == Side::ASK && IsTerminal(executionReport.m_status)) {
      position.m_askQuantityPending -= orderEntry.m_remainingQuantity;
    }
  }

  inline ShortingTracker::PositionEntry& ShortingTracker::GetPosition(
      const Security& security) {
    auto positionIterator = m_securityToPositionEntry.find(security);
    if(positionIterator == m_securityToPositionEntry.end()) {
      positionIterator = m_securityToPositionEntry.insert(
        std::pair(security, PositionEntry())).first;
    }
    return positionIterator->second;
  }
}

#endif
