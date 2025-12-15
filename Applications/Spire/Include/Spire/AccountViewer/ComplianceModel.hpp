#ifndef SPIRE_COMPLIANCEMODEL_HPP
#define SPIRE_COMPLIANCEMODEL_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/signal.hpp>
#include <QString>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"

namespace Spire {
  class UserProfile;

  /*! \class ComplianceModel
      \brief Stores an account's compliance parameters.
   */
  class ComplianceModel {
    public:

      //! Signals a new ComplianceRuleEntry was added.
      /*!
        \param entry The new ComplianceRuleEntry.
      */
      using ComplianceRuleEntryAddedSignal =
        boost::signals2::signal<void (const Nexus::ComplianceRuleEntry& entry)>;

      //! Signals a ComplianceRuleEntry was removed.
      /*!
        \param entry The ComplianceRuleEntry that was removed.
      */
      using ComplianceRuleEntryRemovedSignal =
        boost::signals2::signal<void (const Nexus::ComplianceRuleEntry& entry)>;

      //! Signals a ComplianceRuleEntry's id was changed.
      /*!
        \param previousId The entry's previous id.
        \param newId The entry's new id.
      */
      using ComplianceRuleEntryIdUpdatedSignal = boost::signals2::signal<
        void (Nexus::ComplianceRuleEntry::Id previousId,
          Nexus::ComplianceRuleEntry::Id newId)>;

      //! Constructs a ComplianceModel.
      /*!
        \param userProfile The user's profile.
        \param account The account being represented.
      */
      ComplianceModel(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& account);

      //! Loads the account's compliance model.
      void Load();

      //! Commits the compliance model.
      void Commit();

      //! Returns the account being represented.
      const Beam::DirectoryEntry& GetAccount() const;

      //! Returns the list of schemas available.
      const std::vector<Nexus::ComplianceRuleSchema>& GetSchemas() const;

      //! Returns the list of ComplianceRuleEntries for the account.
      const std::vector<Nexus::ComplianceRuleEntry>& GetEntries() const;

      //! Adds a new ComplianceRuleEntry.
      /*!
        \param schema The entry's schema.
      */
      void Add(const Nexus::ComplianceRuleSchema& schema);

      //! Removes a ComplianceRuleEntry.
      /*!
        \param id The id of the entry to remove.
      */
      void Remove(Nexus::ComplianceRuleEntry::Id id);

      //! Updates a ComplianceEntryRule.
      /*!
        \param entry The updated ComplianceEntryRule.
      */
      void Update(const Nexus::ComplianceRuleEntry& entry);

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
      Beam::DirectoryEntry m_account;
      std::vector<Nexus::ComplianceRuleSchema> m_schemas;
      std::vector<Nexus::ComplianceRuleEntry> m_entries;
      Nexus::ComplianceRuleEntry::Id m_newComplianceEntryId;
      Nexus::ComplianceRuleEntry::Id m_nextComplianceEntryId;
      std::vector<Nexus::ComplianceRuleEntry> m_updatedEntries;
      std::vector<Nexus::ComplianceRuleEntry> m_newEntries;
      std::vector<Nexus::ComplianceRuleEntry::Id> m_deletedEntries;
      mutable ComplianceRuleEntryAddedSignal m_complianceRuleEntryAddedSignal;
      mutable ComplianceRuleEntryAddedSignal m_complianceRuleEntryRemovedSignal;
      mutable ComplianceRuleEntryIdUpdatedSignal
        m_complianceRuleEntryIdUpdatedSignal;
  };

  /** Returns <code>true</code> iff a schema is wrapped. */
  bool IsWrapped(const Nexus::ComplianceRuleSchema& schema);

  /** Returns the name of the fully unwrapped schema. */
  QString GetUnwrappedName(const Nexus::ComplianceRuleSchema& schema);
}

#endif
