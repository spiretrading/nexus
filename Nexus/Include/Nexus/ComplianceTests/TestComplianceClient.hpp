#ifndef NEXUS_TEST_COMPLIANCE_CLIENT_HPP
#define NEXUS_TEST_COMPLIANCE_CLIENT_HPP
#include <memory>
#include <variant>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Async.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include "Nexus/Compliance/ComplianceClient.hpp"

namespace Nexus::Tests {

  /**
   * Implements a ComplianceClient for testing by pushing all operations
   * performed onto a queue.
   */
  class TestComplianceClient {
    public:

      /** Records a call to load(). */
      struct LoadOperation {
        Beam::ServiceLocator::DirectoryEntry m_directory_entry;
        Beam::Services::Tests::ServiceResult<std::vector<ComplianceRuleEntry>>
          m_result;
      };

      /** Records a call to add(). */
      struct AddOperation {
        Beam::ServiceLocator::DirectoryEntry m_directory_entry;
        ComplianceRuleEntry::State m_state;
        ComplianceRuleSchema m_schema;
        Beam::Services::Tests::ServiceResult<ComplianceRuleEntry::Id> m_result;
      };

      /** Records a call to update(). */
      struct UpdateOperation {
        ComplianceRuleEntry m_entry;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to remove(). */
      struct RemoveOperation {
        ComplianceRuleEntry::Id m_id;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to report(). */
      struct ReportOperation {
        ComplianceRuleViolationRecord m_record;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to monitor_compliance_rule_entries(). */
      struct MonitorComplianceRuleEntriesOperation {
        Beam::ServiceLocator::DirectoryEntry m_directory_entry;
        Beam::ScopedQueueWriter<ComplianceRuleEntry> m_queue;
        Beam::Services::Tests::ServiceResult<std::vector<ComplianceRuleEntry>>
          m_result;
      };

      /** A variant covering all possible TestComplianceClient operations. */
      using Operation = std::variant<LoadOperation, AddOperation,
        UpdateOperation, RemoveOperation, ReportOperation,
        MonitorComplianceRuleEntriesOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestComplianceClient.
       * @param operations The queue to push all operations on.
       */
      explicit TestComplianceClient(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
          noexcept;

      ~TestComplianceClient();

      std::vector<ComplianceRuleEntry> load(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);

      ComplianceRuleEntry::Id add(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      void update(const ComplianceRuleEntry& entry);

      void remove(ComplianceRuleEntry::Id id);

      void report(const ComplianceRuleViolationRecord& record);

      void monitor_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void close();

    private:
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::SynchronizedUnorderedSet<Beam::Services::Tests::BaseServiceResult*>
        m_pending_results;
      Beam::IO::OpenState m_open_state;

      TestComplianceClient(const TestComplianceClient&) = delete;
      TestComplianceClient& operator=(const TestComplianceClient&) = delete;
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestComplianceClient::TestComplianceClient(
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestComplianceClient::~TestComplianceClient() {
    close();
  }

  inline std::vector<ComplianceRuleEntry> TestComplianceClient::load(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return append_result<LoadOperation, std::vector<ComplianceRuleEntry>>(
      directory_entry);
  }

  inline ComplianceRuleEntry::Id TestComplianceClient::add(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return append_result<AddOperation, ComplianceRuleEntry::Id>(
      directory_entry, state, schema);
  }

  inline void TestComplianceClient::update(const ComplianceRuleEntry& entry) {
    append_result<UpdateOperation, void>(entry);
  }

  inline void TestComplianceClient::remove(ComplianceRuleEntry::Id id) {
    append_result<RemoveOperation, void>(id);
  }

  inline void TestComplianceClient::report(
      const ComplianceRuleViolationRecord& record) {
    append_result<ReportOperation, void>(record);
  }

  inline void TestComplianceClient::monitor_compliance_rule_entries(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto async = Beam::Routines::Async<std::vector<ComplianceRuleEntry>>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<MonitorComplianceRuleEntriesOperation>,
      directory_entry, std::move(queue), async.GetEval());
    m_pending_results.Insert(&std::get<MonitorComplianceRuleEntriesOperation>(
      *operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(
        &std::get<MonitorComplianceRuleEntriesOperation>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    *snapshot = std::move(async.Get());
    m_pending_results.Erase(&std::get<MonitorComplianceRuleEntriesOperation>(
      *operation).m_result);
  }

  inline void TestComplianceClient::close() {
    if(m_open_state.SetClosing()) {
      m_pending_results.With([&] (auto& results) {
        for(auto& result : results) {
          result->set(std::make_exception_ptr(Beam::IO::EndOfFileException()));
        }
      });
      m_pending_results.Clear();
    }
    m_open_state.Close();
  }

  template<typename T, typename R, typename... Args>
  R TestComplianceClient::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    m_pending_results.Insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(&std::get<T>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    auto result = std::move(async.Get());
    m_pending_results.Erase(&std::get<T>(*operation).m_result);
    if constexpr(!std::is_same_v<R, void>) {
      return result;
    }
  }
}

#endif
