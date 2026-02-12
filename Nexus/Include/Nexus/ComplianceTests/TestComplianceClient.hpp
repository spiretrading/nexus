#ifndef NEXUS_TEST_COMPLIANCE_CLIENT_HPP
#define NEXUS_TEST_COMPLIANCE_CLIENT_HPP
#include <memory>
#include <variant>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
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
        Beam::DirectoryEntry m_directory_entry;
        Beam::Tests::ServiceResult<std::vector<ComplianceRuleEntry>> m_result;
      };

      /** Records a call to add(). */
      struct AddOperation {
        Beam::DirectoryEntry m_directory_entry;
        ComplianceRuleEntry::State m_state;
        ComplianceRuleSchema m_schema;
        Beam::Tests::ServiceResult<ComplianceRuleEntry::Id> m_result;
      };

      /** Records a call to update(). */
      struct UpdateOperation {
        ComplianceRuleEntry m_entry;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to remove(). */
      struct RemoveOperation {
        ComplianceRuleEntry::Id m_id;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to report(). */
      struct ReportOperation {
        ComplianceRuleViolationRecord m_record;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to monitor_compliance_rule_entries(). */
      struct MonitorComplianceRuleEntriesOperation {
        Beam::DirectoryEntry m_directory_entry;
        Beam::ScopedQueueWriter<ComplianceRuleEntry> m_queue;
        Beam::Tests::ServiceResult<std::vector<ComplianceRuleEntry>> m_result;
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
        const Beam::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id add(const Beam::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void update(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void report(const ComplianceRuleViolationRecord& record);
      void monitor_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_queue;

      TestComplianceClient(const TestComplianceClient&) = delete;
      TestComplianceClient& operator=(const TestComplianceClient&) = delete;
  };

  inline TestComplianceClient::TestComplianceClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_queue(std::move(operations)) {}

  inline TestComplianceClient::~TestComplianceClient() {
    close();
  }

  inline std::vector<ComplianceRuleEntry> TestComplianceClient::load(
      const Beam::DirectoryEntry& directory_entry) {
    return m_queue.append_result<
      LoadOperation, std::vector<ComplianceRuleEntry>>(directory_entry);
  }

  inline ComplianceRuleEntry::Id TestComplianceClient::add(
      const Beam::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_queue.append_result<AddOperation, ComplianceRuleEntry::Id>(
      directory_entry, state, schema);
  }

  inline void TestComplianceClient::update(const ComplianceRuleEntry& entry) {
    m_queue.append_result<UpdateOperation, void>(entry);
  }

  inline void TestComplianceClient::remove(ComplianceRuleEntry::Id id) {
    m_queue.append_result<RemoveOperation, void>(id);
  }

  inline void TestComplianceClient::report(
      const ComplianceRuleViolationRecord& record) {
    m_queue.append_result<ReportOperation, void>(record);
  }

  inline void TestComplianceClient::monitor_compliance_rule_entries(
      const Beam::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    *snapshot = m_queue.append_result<MonitorComplianceRuleEntriesOperation,
      std::vector<ComplianceRuleEntry>>(directory_entry, std::move(queue));
  }

  inline void TestComplianceClient::close() {
    m_queue.close();
  }
}

#endif
