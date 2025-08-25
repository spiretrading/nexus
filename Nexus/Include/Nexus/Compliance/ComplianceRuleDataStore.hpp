#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/optional/optional.hpp>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {

  /** Provides a data store for all compliance rules and violations. */
  class ComplianceRuleDataStore {
    public:

      /**
       * Constructs a ComplianceRuleDataStore of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit ComplianceRuleDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ComplianceRuleDataStore by copying an existing data store.
       * @param data_store The data store to copy.
       */
      template<typename DataStore>
      explicit ComplianceRuleDataStore(DataStore data_store);

      explicit ComplianceRuleDataStore(
        ComplianceRuleDataStore* data_store);

      explicit ComplianceRuleDataStore(
        const std::shared_ptr<ComplianceRuleDataStore>& data_store);

      explicit ComplianceRuleDataStore(
        const std::unique_ptr<ComplianceRuleDataStore>& data_store);

      /** Returns all ComplianceRuleEntry records. */
      std::vector<ComplianceRuleEntry> load_all_compliance_rule_entries();

      /** Loads the next id to use for a ComplianceRuleEntry. */
      ComplianceRuleEntry::Id load_next_compliance_rule_entry_id();

      /**
       * Loads a ComplianceRuleEntry from its id.
       * @param id The id of the ComplianceRuleEntry to load.
       * @return The ComplianceRuleEntry with the specified id, or none if not
       *         found.
       */
      boost::optional<ComplianceRuleEntry>
        load_compliance_rule_entry(ComplianceRuleEntry::Id id);

      /**
       * Loads all ComplianceRuleEntry records for a specified DirectoryEntry.
       * @param directory_entry The DirectoryEntry to search for.
       * @return The list of ComplianceRuleEntry records assigned to the
       *         specified directory_entry.
       */
      std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);

      /**
       * Stores a ComplianceRuleEntry.
       * @param entry The ComplianceRuleEntry to store.
       */
      void store(const ComplianceRuleEntry& entry);

      /**
       * Deletes a ComplianceRuleEntry.
       * @param id The id of the ComplianceRuleEntry to delete.
       */
      void remove(ComplianceRuleEntry::Id id);

      /**
       * Stores a ComplianceRuleViolationRecord.
       * @param record The violation to store.
       */
      void store(const ComplianceRuleViolationRecord& record);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualComplianceRuleDataStore {
        virtual ~VirtualComplianceRuleDataStore() = default;
        virtual std::vector<ComplianceRuleEntry>
          load_all_compliance_rule_entries() = 0;
        virtual ComplianceRuleEntry::Id
          load_next_compliance_rule_entry_id() = 0;
        virtual boost::optional<ComplianceRuleEntry>
          load_compliance_rule_entry(ComplianceRuleEntry::Id id) = 0;
        virtual std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry) = 0;
        virtual void store(const ComplianceRuleEntry& entry) = 0;
        virtual void remove(ComplianceRuleEntry::Id id) = 0;
        virtual void store(const ComplianceRuleViolationRecord& record) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedComplianceRuleDataStore final :
          VirtualComplianceRuleDataStore {
        using DataStore = D;
        Beam::GetOptionalLocalPtr<DataStore> m_data_store;
        template<typename... Args>
        WrappedComplianceRuleDataStore(Args&&... args);
        std::vector<ComplianceRuleEntry>
          load_all_compliance_rule_entries() override;
        ComplianceRuleEntry::Id load_next_compliance_rule_entry_id() override;
        boost::optional<ComplianceRuleEntry>
          load_compliance_rule_entry(ComplianceRuleEntry::Id id) override;
        std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry) override;
        void store(const ComplianceRuleEntry& entry) override;
        void remove(ComplianceRuleEntry::Id id) override;
        void store(const ComplianceRuleViolationRecord& record) override;
        void close() override;
      };
      std::shared_ptr<VirtualComplianceRuleDataStore> m_data_store;
  };

  /** Checks if a type implements a ComplianceRuleDataStore. */
  template<typename T>
  concept IsComplianceRuleDataStore = std::constructible_from<
    ComplianceRuleDataStore, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  ComplianceRuleDataStore::ComplianceRuleDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(std::make_shared<WrappedComplianceRuleDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  ComplianceRuleDataStore::ComplianceRuleDataStore(DataStore data_store)
    : ComplianceRuleDataStore(
        std::in_place_type<DataStore>, std::move(data_store)) {}

  inline ComplianceRuleDataStore::ComplianceRuleDataStore(
    ComplianceRuleDataStore* data_store)
    : ComplianceRuleDataStore(*data_store) {}

  inline ComplianceRuleDataStore::ComplianceRuleDataStore(
      const std::shared_ptr<ComplianceRuleDataStore>& data_store)
    : ComplianceRuleDataStore(*data_store) {}

  inline ComplianceRuleDataStore::ComplianceRuleDataStore(
      const std::unique_ptr<ComplianceRuleDataStore>& data_store)
    : ComplianceRuleDataStore(*data_store) {}

  inline std::vector<ComplianceRuleEntry>
      ComplianceRuleDataStore::load_all_compliance_rule_entries() {
    return m_data_store->load_all_compliance_rule_entries();
  }

  inline ComplianceRuleEntry::Id
      ComplianceRuleDataStore::load_next_compliance_rule_entry_id() {
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  inline boost::optional<ComplianceRuleEntry>
      ComplianceRuleDataStore::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    return m_data_store->load_compliance_rule_entry(id);
  }

  inline std::vector<ComplianceRuleEntry>
      ComplianceRuleDataStore::load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  inline void ComplianceRuleDataStore::store(const ComplianceRuleEntry& entry) {
    m_data_store->store(entry);
  }

  inline void ComplianceRuleDataStore::remove(ComplianceRuleEntry::Id id) {
    m_data_store->remove(id);
  }

  inline void ComplianceRuleDataStore::store(
      const ComplianceRuleViolationRecord& record) {
    m_data_store->store(record);
  }

  inline void ComplianceRuleDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  ComplianceRuleDataStore::WrappedComplianceRuleDataStore<D>::
    WrappedComplianceRuleDataStore(Args&&... args)
      : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<ComplianceRuleEntry> ComplianceRuleDataStore::
      WrappedComplianceRuleDataStore<D>::load_all_compliance_rule_entries() {
    return m_data_store->load_all_compliance_rule_entries();
  }

  template<typename D>
  ComplianceRuleEntry::Id ComplianceRuleDataStore::
      WrappedComplianceRuleDataStore<D>::load_next_compliance_rule_entry_id() {
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  template<typename D>
  boost::optional<ComplianceRuleEntry> ComplianceRuleDataStore::
      WrappedComplianceRuleDataStore<D>::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    return m_data_store->load_compliance_rule_entry(id);
  }

  template<typename D>
  std::vector<ComplianceRuleEntry> ComplianceRuleDataStore::
      WrappedComplianceRuleDataStore<D>::load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename D>
  void ComplianceRuleDataStore::WrappedComplianceRuleDataStore<D>::store(
      const ComplianceRuleEntry& entry) {
    m_data_store->store(entry);
  }

  template<typename D>
  void ComplianceRuleDataStore::WrappedComplianceRuleDataStore<D>::remove(
      ComplianceRuleEntry::Id id) {
    m_data_store->remove(id);
  }

  template<typename D>
  void ComplianceRuleDataStore::WrappedComplianceRuleDataStore<D>::store(
      const ComplianceRuleViolationRecord& record) {
    m_data_store->store(record);
  }

  template<typename D>
  void ComplianceRuleDataStore::WrappedComplianceRuleDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
