#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {

  /** Checks if a type implements a ComplianceRuleDataStore. */
  template<typename T>
  concept IsComplianceRuleDataStore =
    Beam::IsConnection<T> && requires(T& data_store) {
      { data_store.load_all_compliance_rule_entries() } ->
          std::same_as<std::vector<ComplianceRuleEntry>>;
      { data_store.load_next_compliance_rule_entry_id() } ->
          std::same_as<ComplianceRuleEntry::Id>;
      { data_store.load_compliance_rule_entry(
          std::declval<ComplianceRuleEntry::Id>()) } ->
            std::same_as<boost::optional<ComplianceRuleEntry>>;
      { data_store.load_compliance_rule_entries(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<std::vector<ComplianceRuleEntry>>;
      data_store.store(std::declval<const ComplianceRuleEntry&>());
      data_store.remove(std::declval<ComplianceRuleEntry::Id>());
      data_store.store(std::declval<const ComplianceRuleViolationRecord&>());
    };

  /** Provides a data store for all compliance rules and violations. */
  class ComplianceRuleDataStore {
    public:

      /**
       * Constructs a ComplianceRuleDataStore of a specified type using
       * emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsComplianceRuleDataStore T, typename... Args>
      explicit ComplianceRuleDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ComplianceRuleDataStore by referencing an existing data
       * store.
       * @param data_store The data store to reference.
       */
      template<Beam::DisableCopy<ComplianceRuleDataStore> T> requires
        IsComplianceRuleDataStore<Beam::dereference_t<T>>
      ComplianceRuleDataStore(T&& data_store);

      ComplianceRuleDataStore(const ComplianceRuleDataStore&) = default;
      ComplianceRuleDataStore(ComplianceRuleDataStore&&) = default;

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
        const Beam::DirectoryEntry& directory_entry);

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
          const Beam::DirectoryEntry& directory_entry) = 0;
        virtual void store(const ComplianceRuleEntry& entry) = 0;
        virtual void remove(ComplianceRuleEntry::Id id) = 0;
        virtual void store(const ComplianceRuleViolationRecord& record) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedComplianceRuleDataStore final :
          VirtualComplianceRuleDataStore {
        using DataStore = D;
        Beam::local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedComplianceRuleDataStore(Args&&... args);

        std::vector<ComplianceRuleEntry>
          load_all_compliance_rule_entries() override;
        ComplianceRuleEntry::Id load_next_compliance_rule_entry_id() override;
        boost::optional<ComplianceRuleEntry>
          load_compliance_rule_entry(ComplianceRuleEntry::Id id) override;
        std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
          const Beam::DirectoryEntry& directory_entry) override;
        void store(const ComplianceRuleEntry& entry) override;
        void remove(ComplianceRuleEntry::Id id) override;
        void store(const ComplianceRuleViolationRecord& record) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualComplianceRuleDataStore> m_data_store;
  };

  template<IsComplianceRuleDataStore T, typename... Args>
  ComplianceRuleDataStore::ComplianceRuleDataStore(
      std::in_place_type_t<T>, Args&&... args)
    : m_data_store(Beam::make_virtual_ptr<WrappedComplianceRuleDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<ComplianceRuleDataStore> T> requires
    IsComplianceRuleDataStore<Beam::dereference_t<T>>
  ComplianceRuleDataStore::ComplianceRuleDataStore(T&& data_store)
    : m_data_store(Beam::make_virtual_ptr<WrappedComplianceRuleDataStore<
        std::remove_cvref_t<T>>>(std::forward<T>(data_store))) {}

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
        const Beam::DirectoryEntry& directory_entry) {
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
        const Beam::DirectoryEntry& directory_entry) {
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
