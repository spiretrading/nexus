#ifndef SPIRE_COMPLIANCEMODEL_HPP
#define SPIRE_COMPLIANCEMODEL_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class ComplianceModel
      \brief Stores an account's compliance parameters.
   */
  class ComplianceModel {
    public:

      //! Signals a new ComplianceRuleEntry was added.
      /*!
        \param entry The new ComplianceRuleEntry.
      */
      using ComplianceRuleEntryAddedSignal = boost::signals2::signal<
        void (const Nexus::Compliance::ComplianceRuleEntry& entry)>;

      //! Signals a ComplianceRuleEntry was removed.
      /*!
        \param entry The ComplianceRuleEntry that was removed.
      */
      using ComplianceRuleEntryRemovedSignal = boost::signals2::signal<
        void (const Nexus::Compliance::ComplianceRuleEntry& entry)>;

      //! Signals a ComplianceRuleEntry's id was changed.
      /*!
        \param previousId The entry's previous id.
        \param newId The entry's new id.
      */
      using ComplianceRuleEntryIdUpdatedSignal = boost::signals2::signal<
        void (Nexus::Compliance::ComplianceRuleId previousId,
        Nexus::Compliance::ComplianceRuleId newId)>;

      //! Constructs a ComplianceModel.
      /*!
        \param userProfile The user's profile.
        \param account The account being represented.
      */
      ComplianceModel(Beam::Ref<UserProfile> userProfile,
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Loads the account's compliance model.
      void Load();

      //! Commits the compliance model.
      void Commit();

      //! Returns the account being represented.
      const Beam::ServiceLocator::DirectoryEntry& GetAccount() const;

      //! Returns the list of schemas available.
      const std::vector<
        Nexus::Compliance::ComplianceRuleSchema>& GetSchemas() const;

      //! Returns the list of ComplianceRuleEntries for the account.
      const std::vector<
        Nexus::Compliance::ComplianceRuleEntry>& GetEntries() const;

      //! Adds a new ComplianceRuleEntry.
      /*!
        \param schema The entry's schema.
      */
      void Add(const Nexus::Compliance::ComplianceRuleSchema& schema);

      //! Removes a ComplianceRuleEntry.
      /*!
        \param id The id of the entry to remove.
      */
      void Remove(Nexus::Compliance::ComplianceRuleId id);

      //! Updates a ComplianceEntryRule.
      /*!
        \param entry The updated ComplianceEntryRule.
      */
      void Update(const Nexus::Compliance::ComplianceRuleEntry& entry);

      //! Connects a slot to the ComplianceRuleEntryAddedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the slot.
      */
      boost::signals2::connection ConnectComplianceRuleEntryAddedSignal(
        const ComplianceRuleEntryAddedSignal::slot_type& slot) const;

      //! Connects a slot to the ComplianceRuleEntryRemovedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the slot.
      */
      boost::signals2::connection ConnectComplianceRuleEntryRemovedSignal(
        const ComplianceRuleEntryRemovedSignal::slot_type& slot) const;

      //! Connects a slot to the ComplianceRuleEntryIdUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the slot.
      */
      boost::signals2::connection ConnectComplianceRuleEntryIdUpdatedSignal(
        const ComplianceRuleEntryIdUpdatedSignal::slot_type& slot) const;

    private:
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_account;
      std::vector<Nexus::Compliance::ComplianceRuleSchema> m_schemas;
      std::vector<Nexus::Compliance::ComplianceRuleEntry> m_entries;
      Nexus::Compliance::ComplianceRuleId m_newComplianceEntryId;
      Nexus::Compliance::ComplianceRuleId m_nextComplianceEntryId;
      std::vector<Nexus::Compliance::ComplianceRuleEntry> m_updatedEntries;
      std::vector<Nexus::Compliance::ComplianceRuleEntry> m_newEntries;
      std::vector<Nexus::Compliance::ComplianceRuleId> m_deletedEntries;
      mutable ComplianceRuleEntryAddedSignal m_complianceRuleEntryAddedSignal;
      mutable ComplianceRuleEntryAddedSignal m_complianceRuleEntryRemovedSignal;
      mutable ComplianceRuleEntryIdUpdatedSignal
        m_complianceRuleEntryIdUpdatedSignal;
  };
}

#endif
