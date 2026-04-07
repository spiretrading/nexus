#ifndef NEXUS_COMPLIANCE_RULE_SET_HPP
#define NEXUS_COMPLIANCE_RULE_SET_HPP
#include <deque>
#include <memory>
#include <unordered_set>
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/Active.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Validates an Order operation against a set of ComplianceRuleEntries.
   * @param <C> The type of ComplianceClient to use.
   * @param <S> The type of ServiceLocatorClient used to lookup DirectoryEntries
   *        for accounts and their parents.
   */
  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  class ComplianceRuleSet {
    public:

      /** The type of ComplianceClient to use. */
      using ComplianceClient = Beam::dereference_t<C>;

      /**
       * The type of ServiceLocatorClient used to lookup DirectoryEntries for
       * accounts and their parents.
       */
      using ServiceLocatorClient = Beam::dereference_t<S>;

      /**
       * Returns a ComplianceRule from a ComplianceRuleEntry.
       * @param entry The ComplianceRuleEntry to build the rule from.
       * @return The ComplianceRule represented by the <i>entry</i>.
       */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleEntry& entry)>;

      /**
       * Constructs a ComplianceRuleSet.
       * @param compliance_client Initializes the ComplianceClient.
       * @param service_locator_client Initializes The ServiceLocatorClient.
       * @param builder Constructs compliance rules from a ComplianceRuleEntry.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<S> SF>
      ComplianceRuleSet(CF&& compliance_client, SF&& service_locator_client,
        ComplianceRuleBuilder builder);

      /**
       * Performs a compliance check on an Order submission.
       * @param order The Order being submitted.
       */
      void submit(const std::shared_ptr<Order>& order);

      /**
       * Cancels a previously submitted Order.
       * @param account The account submitting the cancel request.
       * @param order The Order being canceled.
       */
      void cancel(const Beam::DirectoryEntry& account,
        const std::shared_ptr<Order>& order);

      /**
       * Adds an Order that successfully passed all compliance checks.
       * @param order The Order that was successfully submitted.
       */
      void add(const std::shared_ptr<Order>& order);

    private:
      struct Rule {
        Beam::Active<ComplianceRuleEntry> m_entry;
        std::unique_ptr<ComplianceRule> m_rule;
      };
      struct Entry {
        Beam::Mutex m_mutex;
        std::vector<Beam::DirectoryEntry> m_parents;
        std::vector<std::shared_ptr<Rule>> m_rules;
        std::vector<std::shared_ptr<Order>> m_orders;
        Beam::CallOnce<Beam::Mutex> m_initializer;
      };
      Beam::local_ptr_t<C> m_compliance_client;
      Beam::local_ptr_t<S> m_service_locator_client;
      Beam::SynchronizedUnorderedMap<
        Beam::DirectoryEntry, std::shared_ptr<Entry>> m_entries;
      ComplianceRuleBuilder m_builder;
      Beam::RoutineTaskQueue m_tasks;

      ComplianceRuleSet(const ComplianceRuleSet&) = delete;
      ComplianceRuleSet& operator =(const ComplianceRuleSet&) = delete;
      std::shared_ptr<Entry> load(
        const Beam::DirectoryEntry& directory_entry);
      void update(const ComplianceRuleEntry& updated_entry, Entry& entry);
      void on_compliance_update(const ComplianceRuleEntry& updated_entry);
  };

  template<typename C, typename S>
  ComplianceRuleSet(C&&, S&&, std::function<
    std::unique_ptr<ComplianceRule> (const ComplianceRuleEntry&)>) ->
      ComplianceRuleSet<std::remove_cvref_t<C>, std::remove_cvref_t<S>>;

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  template<Beam::Initializes<C> CF, Beam::Initializes<S> SF>
  ComplianceRuleSet<C, S>::ComplianceRuleSet(CF&& compliance_client,
      SF&& service_locator_client, ComplianceRuleBuilder builder)
    : m_compliance_client(std::forward<CF>(compliance_client)),
      m_service_locator_client(std::forward<SF>(service_locator_client)),
      m_builder(std::move(builder)) {}

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  void ComplianceRuleSet<C, S>::submit(const std::shared_ptr<Order>& order) {
    auto exception = std::exception_ptr();
    auto entry = load(order->get_info().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      entry->m_orders.push_back(order);
      for(auto& rule : entry->m_rules) {
        auto rule_entry = rule->m_entry.load();
        if(rule_entry->get_state() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->submit(order);
        } catch(const ComplianceCheckException& e) {
          m_compliance_client->report({order->get_info().m_submission_account,
            order->get_info().m_id, rule_entry->get_id(),
            rule_entry->get_schema().get_name(), e.what()});
          if(rule_entry->get_state() == ComplianceRuleEntry::State::ACTIVE) {
            exception = std::current_exception();
            break;
          }
        }
      }
    }
    for(auto& parent : entry->m_parents) {
      auto parent_entry = load(parent);
      auto lock = boost::lock_guard(parent_entry->m_mutex);
      parent_entry->m_orders.push_back(order);
      if(exception) {
        continue;
      }
      for(auto& rule : parent_entry->m_rules) {
        auto rule_entry = rule->m_entry.load();
        if(rule_entry->get_state() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->submit(order);
        } catch(const ComplianceCheckException& e) {
          m_compliance_client->report({order->get_info().m_submission_account,
            order->get_info().m_id, rule_entry->get_id(),
            rule_entry->get_schema().get_name(), e.what()});
          if(rule_entry->get_state() == ComplianceRuleEntry::State::ACTIVE) {
            exception = std::current_exception();
            break;
          }
        }
      }
    }
    if(exception) {
      std::rethrow_exception(exception);
    }
  }

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  void ComplianceRuleSet<C, S>::cancel(
      const Beam::DirectoryEntry& account,
      const std::shared_ptr<Order>& order) {
    auto entry = load(order->get_info().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      for(auto& rule : entry->m_rules) {
        auto rule_entry = rule->m_entry.load();
        if(rule_entry->get_state() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->cancel(order);
        } catch(const ComplianceCheckException& e) {
          m_compliance_client->report({account, order->get_info().m_id,
            rule_entry->get_id(), rule_entry->get_schema().get_name(),
            e.what()});
          if(rule_entry->get_state() == ComplianceRuleEntry::State::ACTIVE) {
            throw;
          }
        }
      }
    }
    for(auto& parent : entry->m_parents) {
      auto parent_entry = load(parent);
      auto lock = boost::lock_guard(parent_entry->m_mutex);
      for(auto& rule : parent_entry->m_rules) {
        auto rule_entry = rule->m_entry.load();
        if(rule_entry->get_state() == ComplianceRuleEntry::State::DISABLED) {
          continue;
        }
        try {
          rule->m_rule->cancel(order);
        } catch(const ComplianceCheckException& e) {
          m_compliance_client->report({account, order->get_info().m_id,
            rule_entry->get_id(), rule_entry->get_schema().get_name(),
            e.what()});
          if(rule_entry->get_state() == ComplianceRuleEntry::State::ACTIVE) {
            throw;
          }
        }
      }
    }
  }

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  void ComplianceRuleSet<C, S>::add(const std::shared_ptr<Order>& order) {
    auto entry = load(order->get_info().m_fields.m_account);
    {
      auto lock = boost::lock_guard(entry->m_mutex);
      entry->m_orders.push_back(order);
      for(auto& rule : entry->m_rules) {
        rule->m_rule->add(order);
      }
    }
    for(const auto& parent : entry->m_parents) {
      auto parent_entry = load(parent);
      auto lock = boost::lock_guard(parent_entry->m_mutex);
      parent_entry->m_orders.push_back(order);
      for(auto& rule : parent_entry->m_rules) {
        rule->m_rule->add(order);
      }
    }
  }

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  std::shared_ptr<typename ComplianceRuleSet<C, S>::Entry>
      ComplianceRuleSet<C, S>::load(
        const Beam::DirectoryEntry& directory_entry) {
    auto entry = m_entries.get_or_insert(
      directory_entry, boost::factory<std::shared_ptr<Entry>>());
    entry->m_initializer.call([&] {
      auto discovered_parents =
        std::unordered_set<Beam::DirectoryEntry>();
      auto queue = std::deque<Beam::DirectoryEntry>();
      queue.push_back(directory_entry);
      while(!queue.empty()) {
        auto current = std::move(queue.front());
        queue.pop_front();
        auto parents = m_service_locator_client->load_parents(current);
        for(auto& parent : parents) {
          if(current.m_type == Beam::DirectoryEntry::Type::ACCOUNT) {
            if(parent.m_name != "traders" && parent.m_name != "managers") {
              continue;
            }
          }
          if(discovered_parents.insert(parent).second) {
            entry->m_parents.push_back(parent);
            queue.push_back(parent);
          }
        }
      }
      auto rules = std::vector<ComplianceRuleEntry>();
      m_compliance_client->monitor_compliance_rule_entries(
        directory_entry, m_tasks.get_slot<ComplianceRuleEntry>(
          std::bind_front(&ComplianceRuleSet::on_compliance_update, this)),
        Beam::out(rules));
      for(auto& rule : rules) {
        update(rule, *entry);
      }
    });
    return entry;
  }

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  void ComplianceRuleSet<C, S>::update(
      const ComplianceRuleEntry& updated_entry, Entry& entry) {
    auto lock = boost::lock_guard(entry.m_mutex);
    entry.m_rules.erase(
      std::remove_if(entry.m_rules.begin(), entry.m_rules.end(),
        [&] (const auto& rule) {
          return rule->m_entry.load()->get_id() == updated_entry.get_id();
        }), entry.m_rules.end());
    if(updated_entry.get_state() == ComplianceRuleEntry::State::DELETED) {
      return;
    }
    if(auto compliance_rule = m_builder(updated_entry)) {
      auto rule = std::make_shared<Rule>();
      rule->m_entry.update(updated_entry);
      rule->m_rule = std::move(compliance_rule);
      for(auto& order : entry.m_orders) {
        rule->m_rule->add(order);
      }
      entry.m_rules.push_back(rule);
    } else {
      std::cerr << "Unknown compliance rule: " <<
        updated_entry.get_schema().get_name() << "\n";
    }
  }

  template<typename C, typename S> requires
    IsComplianceClient<Beam::dereference_t<C>> &&
      Beam::IsServiceLocatorClient<Beam::dereference_t<S>>
  void ComplianceRuleSet<C, S>::on_compliance_update(
      const ComplianceRuleEntry& updated_entry) {
    auto entry = load(updated_entry.get_directory_entry());
    update(updated_entry, *entry);
  }
}

#endif
