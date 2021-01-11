#ifndef NEXUS_COMPLIANCE_RULE_SET_HPP
#define NEXUS_COMPLIANCE_RULE_SET_HPP
#include <deque>
#include <unordered_set>
#include <vector>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/Active.hpp>
#include <Beam/Utilities/Rethrow.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::Compliance {

  /**
   * Validates an Order operation against a set of ComplianceRuleEntries.
   * @param <C> The type of ComplianceClient to use.
   * @param <S> The type of ServiceLocatorClient used to lookup DirectoryEntries
   *        for accounts and their parents.
   */
  template<typename C, typename S>
  class ComplianceRuleSet {
    public:

      /** The type of ComplianceClient to use. */
      using ComplianceClient = Beam::GetTryDereferenceType<C>;

      /**
       * The type of ServiceLocatorClient used to lookup DirectoryEntries for
       * accounts and their parents.
       */
      using ServiceLocatorClient = Beam::GetTryDereferenceType<S>;

      /**
       * Returns a ComplianceRule from a ComplianceRuleEntry.
       * @param entry The ComplianceRuleEntry to build the rule from.
       * @return The ComplianceRule represented by the <i>entry</i>.
       */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleEntry& entry)>;

      /**
       * Constructs a ComplianceRuleSet.
       * @param complianceClient Initializes the ComplianceClient.
       * @param serviceLocatorClient Initializes The ServiceLocatorClient.
       * @param complianceRuleBuilder Constructs compliance rules from a
       *        ComplianceRuleEntry.
       */
      template<typename CF, typename SF>
      ComplianceRuleSet(CF&& complianceClient, SF&& serviceLocatorClient,
        ComplianceRuleBuilder complianceRuleBuilder);

      /**
       * Performs a compliance check on an Order submission.
       * @param order The Order being submitted.
       */
      void Submit(const OrderExecutionService::Order& order);

      /**
       * Cancels a previously submitted Order.
       * @param cancelAccount The account submitting the cancel request.
       * @param order The Order being canceled.
       */
      void Cancel(const Beam::ServiceLocator::DirectoryEntry& cancelAccount,
        const OrderExecutionService::Order& order);

      /**
       * Adds an Order that successfully passed all compliance checks.
       * @param order The Order that was successfully submitted.
       */
      void Add(const OrderExecutionService::Order& order);

    private:
      struct Rule {
        Beam::Active<ComplianceRuleEntry> m_entry;
        std::unique_ptr<ComplianceRule> m_rule;
      };
      struct Entry {
        Beam::Threading::Mutex m_mutex;
        std::vector<Beam::ServiceLocator::DirectoryEntry> m_parents;
        std::vector<std::shared_ptr<Rule>> m_rules;
        std::vector<const OrderExecutionService::Order*> m_orders;
        Beam::Threading::CallOnce<Beam::Threading::Mutex> m_initializer;
      };
      Beam::GetOptionalLocalPtr<C> m_complianceClient;
      Beam::GetOptionalLocalPtr<S> m_serviceLocatorClient;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<Entry>> m_entries;
      ComplianceRuleBuilder m_complianceRuleBuilder;
      Beam::RoutineTaskQueue m_tasks;

      ComplianceRuleSet(const ComplianceRuleSet&) = delete;
      ComplianceRuleSet& operator =(const ComplianceRuleSet&) = delete;
      std::shared_ptr<Entry> LoadEntry(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);
      void UpdateComplianceEntry(const ComplianceRuleEntry& updatedEntry,
        Entry& entry);
      void OnComplianceUpdate(const ComplianceRuleEntry& updatedEntry);
  };

  template<typename C, typename S>
  ComplianceRuleSet(C&&, S&&, std::function<
    std::unique_ptr<ComplianceRule> (const ComplianceRuleEntry&)>) ->
      ComplianceRuleSet<std::remove_reference_t<C>, std::remove_reference_t<S>>;

  template<typename C, typename S>
  template<typename CF, typename SF>
  ComplianceRuleSet<C, S>::ComplianceRuleSet(CF&& complianceClient,
    SF&& serviceLocatorClient, ComplianceRuleBuilder complianceRuleBuilder)
    : m_complianceClient(std::forward<CF>(complianceClient)),
      m_serviceLocatorClient(std::forward<SF>(serviceLocatorClient)),
      m_complianceRuleBuilder(std::move(complianceRuleBuilder)) {}

  template<typename C, typename S>
  void ComplianceRuleSet<C, S>::Submit(
      const OrderExecutionService::Order& order) {
    auto exception = std::exception_ptr();
    auto entry = LoadEntry(order.GetInfo().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      entry->m_orders.push_back(&order);
      for(auto& rule : entry->m_rules) {
        auto ruleEntry = rule->m_entry.Load();
        if(ruleEntry->GetState() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->Submit(order);
        } catch(const ComplianceCheckException& e) {
          m_complianceClient->Report({order.GetInfo().m_submissionAccount,
            order.GetInfo().m_orderId, ruleEntry->GetId(),
            ruleEntry->GetSchema().GetName(), e.what()});
          if(ruleEntry->GetState() == ComplianceRuleEntry::State::ACTIVE) {
            exception = std::current_exception();
            break;
          }
        }
      }
    }
    for(auto& parent : entry->m_parents) {
      auto parentEntry = LoadEntry(parent);
      auto lock = boost::lock_guard(parentEntry->m_mutex);
      parentEntry->m_orders.push_back(&order);
      if(exception != nullptr) {
        continue;
      }
      for(auto& rule : parentEntry->m_rules) {
        auto ruleEntry = rule->m_entry.Load();
        if(ruleEntry->GetState() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->Submit(order);
        } catch(const ComplianceCheckException& e) {
          m_complianceClient->Report({order.GetInfo().m_submissionAccount,
            order.GetInfo().m_orderId, ruleEntry->GetId(),
            ruleEntry->GetSchema().GetName(), e.what()});
          if(ruleEntry->GetState() == ComplianceRuleEntry::State::ACTIVE) {
            exception = std::current_exception();
            break;
          }
        }
      }
    }
    Beam::Rethrow(exception);
  }

  template<typename C, typename S>
  void ComplianceRuleSet<C, S>::Cancel(
      const Beam::ServiceLocator::DirectoryEntry& cancelAccount,
      const OrderExecutionService::Order& order) {
    auto entry = LoadEntry(order.GetInfo().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      for(auto& rule : entry->m_rules) {
        auto ruleEntry = rule->m_entry.Load();
        if(ruleEntry->GetState() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->Cancel(order);
        } catch(const ComplianceCheckException& e) {
          m_complianceClient->Report({cancelAccount, order.GetInfo().m_orderId,
            ruleEntry->GetId(), ruleEntry->GetSchema().GetName(), e.what()});
          if(ruleEntry->GetState() == ComplianceRuleEntry::State::ACTIVE) {
            throw;
          }
        }
      }
    }
    for(auto& parent : entry->m_parents) {
      auto parentEntry = LoadEntry(parent);
      auto lock = boost::lock_guard(parentEntry->m_mutex);
      for(auto& rule : parentEntry->m_rules) {
        auto ruleEntry = rule->m_entry.Load();
        if(ruleEntry->GetState() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->Cancel(order);
        } catch(const ComplianceCheckException& e) {
          m_complianceClient->Report({cancelAccount, order.GetInfo().m_orderId,
            ruleEntry->GetId(), ruleEntry->GetSchema().GetName(), e.what()});
          if(ruleEntry->GetState() == ComplianceRuleEntry::State::ACTIVE) {
            throw;
          }
        }
      }
    }
  }

  template<typename C, typename S>
  void ComplianceRuleSet<C, S>::Add(
      const OrderExecutionService::Order& order) {
    auto entry = LoadEntry(order.GetInfo().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      entry->m_orders.push_back(&order);
      for(auto& rule : entry->m_rules) {
        rule->m_rule->Add(order);
      }
    }
    for(auto& parent : entry->m_parents) {
      auto parentEntry = LoadEntry(parent);
      auto lock = boost::lock_guard(parentEntry->m_mutex);
      parentEntry->m_orders.push_back(&order);
      for(auto& rule : parentEntry->m_rules) {
        rule->m_rule->Add(order);
      }
    }
  }

  template<typename C, typename S>
  std::shared_ptr<typename ComplianceRuleSet<C, S>::Entry>
      ComplianceRuleSet<C, S>::LoadEntry(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto entry = m_entries.GetOrInsert(directoryEntry,
      boost::factory<std::shared_ptr<Entry>>());
    entry->m_initializer.Call([&] {
      auto previousParents =
        std::unordered_set<Beam::ServiceLocator::DirectoryEntry>();
      auto searchQueue = std::deque<Beam::ServiceLocator::DirectoryEntry>();
      searchQueue.push_back(directoryEntry);
      while(!searchQueue.empty()) {
        auto frontEntry = std::move(searchQueue.front());
        searchQueue.pop_front();
        auto parents = m_serviceLocatorClient->LoadParents(frontEntry);
        for(auto& parent : parents) {
          if(frontEntry.m_type ==
              Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
            if(parent.m_name != "traders" && parent.m_name != "managers") {
              continue;
            }
          }
          if(previousParents.insert(parent).second) {
            entry->m_parents.push_back(parent);
            searchQueue.push_back(parent);
          }
        }
      }
      auto rules = std::vector<ComplianceRuleEntry>();
      m_complianceClient->MonitorComplianceRuleEntries(directoryEntry,
        m_tasks.GetSlot<ComplianceRuleEntry>(std::bind(
          &ComplianceRuleSet::OnComplianceUpdate, this, std::placeholders::_1)),
        Beam::Store(rules));
      for(auto& rule : rules) {
        UpdateComplianceEntry(rule, *entry);
      }
    });
    return entry;
  }

  template<typename C, typename S>
  void ComplianceRuleSet<C, S>::UpdateComplianceEntry(
      const ComplianceRuleEntry& updatedEntry, Entry& entry) {
    auto lock = boost::lock_guard(entry.m_mutex);
    entry.m_rules.erase(std::remove_if(entry.m_rules.begin(),
      entry.m_rules.end(), [&] (const auto& rule) {
        return rule->m_entry.Load()->GetId() == updatedEntry.GetId();
      }), entry.m_rules.end());
    if(updatedEntry.GetState() == ComplianceRuleEntry::State::DELETED) {
      return;
    }
    if(auto complianceRule = m_complianceRuleBuilder(updatedEntry)) {
      auto rule = std::make_shared<Rule>();
      rule->m_entry.Update(updatedEntry);
      rule->m_rule = std::move(complianceRule);
      for(auto& order : entry.m_orders) {
        rule->m_rule->Add(*order);
      }
      entry.m_rules.push_back(rule);
    } else {
      std::cerr << "Unknown compliance rule: " <<
        updatedEntry.GetSchema().GetName() << "\n";
    }
  }

  template<typename C, typename S>
  void ComplianceRuleSet<C, S>::OnComplianceUpdate(
      const ComplianceRuleEntry& updatedEntry) {
    auto entry = LoadEntry(updatedEntry.GetDirectoryEntry());
    UpdateComplianceEntry(updatedEntry, *entry);
  }
}

#endif
