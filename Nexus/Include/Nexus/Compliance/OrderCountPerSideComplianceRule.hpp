#ifndef NEXUS_ORDERCOUNTPERSIDECOMPLIANCERULE_HPP
#define NEXUS_ORDERCOUNTPERSIDECOMPLIANCERULE_HPP
#include <vector>
#include <boost/atomic/atomic.hpp>
#include <boost/functional/factory.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class OrderCountPerSideComplianceRule
      \brief Restricts the number of Orders per Side per Security.
   */
  class OrderCountPerSideComplianceRule : public ComplianceRule {
    public:

      //! Constructs a OrderCountPerSideComplianceRule.
      /*!
        \param parameters The list of parameters used by this rule.
      */
      OrderCountPerSideComplianceRule(
        const std::vector<ComplianceParameter>& parameters);

      virtual void Submit(const OrderExecutionService::Order& order);

      virtual void Add(const OrderExecutionService::Order& order);

    private:
      SecuritySet m_securities;
      int m_count;
      Beam::SynchronizedUnorderedMap<Security, boost::atomic_int> m_askCounts;
      Beam::SynchronizedUnorderedMap<Security, boost::atomic_int> m_bidCounts;
      Beam::RoutineTaskQueue m_tasks;

      void OnExecutionReport(const Security& security, Side side,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  //! Builds a ComplianceRuleSchema representing an
  //! OrderCountPerSideComplianceRule.
  inline ComplianceRuleSchema BuildOrderCountPerSideComplianceRuleSchema() {
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{});
    std::vector<ComplianceParameter> parameters;
    parameters.emplace_back("symbols", symbols);
    parameters.emplace_back("count", Quantity{0});
    ComplianceRuleSchema schema{"orders_per_side_limit", parameters};
    return schema;
  }

  inline OrderCountPerSideComplianceRule::OrderCountPerSideComplianceRule(
      const std::vector<ComplianceParameter>& parameters)
      : m_count{0} {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          m_securities.Add(std::move(boost::get<Security>(security)));
        }
      } else if(parameter.m_name == "count") {
        m_count = static_cast<int>(boost::get<Quantity>(parameter.m_value));
      }
    }
  }

  inline void OrderCountPerSideComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(!m_securities.Contains(order.GetInfo().m_fields.m_security)) {
      return;
    }
    auto& orderCounts = Pick(order.GetInfo().m_fields.m_side, m_askCounts,
      m_bidCounts);
    auto& orderCount = orderCounts.GetOrInsert(
      order.GetInfo().m_fields.m_security,
      [] {
        return 0;
      });
    auto currentOrderCount = ++orderCount;
    if(currentOrderCount > m_count) {
      --orderCount;
      throw ComplianceCheckException{"Order limit per side reached."};
    } else {
      order.GetPublisher().Monitor(
        m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
        std::bind(&OrderCountPerSideComplianceRule::OnExecutionReport, this,
        order.GetInfo().m_fields.m_security, order.GetInfo().m_fields.m_side,
        std::placeholders::_1)));
    }
  }

  inline void OrderCountPerSideComplianceRule::Add(
      const OrderExecutionService::Order& order) {
    if(!m_securities.Contains(order.GetInfo().m_fields.m_security)) {
      return;
    }
    auto& orderCounts = Pick(order.GetInfo().m_fields.m_side, m_askCounts,
      m_bidCounts);
    auto& orderCount = orderCounts.GetOrInsert(
      order.GetInfo().m_fields.m_security,
      [] {
        return 0;
      });
    ++orderCount;
    order.GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
      std::bind(&OrderCountPerSideComplianceRule::OnExecutionReport, this,
      order.GetInfo().m_fields.m_security, order.GetInfo().m_fields.m_side,
      std::placeholders::_1)));
  }

  inline void OrderCountPerSideComplianceRule::OnExecutionReport(
      const Security& security, Side side,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(!IsTerminal(executionReport.m_status)) {
      return;
    }
    auto& orderCounts = Pick(side, m_askCounts, m_bidCounts);
    auto& orderCount = orderCounts.GetOrInsert(security,
      [] {
        return 0;
      });
    --orderCount;
  }
}
}

#endif
