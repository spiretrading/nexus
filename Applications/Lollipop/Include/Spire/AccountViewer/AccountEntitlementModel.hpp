#ifndef SPIRE_ACCOUNTENTITLEMENTMODEL_HPP
#define SPIRE_ACCOUNTENTITLEMENTMODEL_HPP
#include <unordered_set>
#include <boost/signals2/signal.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class AccountEntitlementModel
      \brief Stores an account's entitlements.
   */
  class AccountEntitlementModel {
    public:

      //! Signals that an entitlement has been granted.
      /*!
        \param entry The group entry of the entitlement granted.
      */
      typedef boost::signals2::signal<
        void (const Beam::DirectoryEntry& entry)>
        EntitlementGrantedSignal;

      //! Signals that an entitlement has been revoked.
      /*!
        \param entry The group entry of the revoked entitlement.
      */
      typedef boost::signals2::signal<
        void (const Beam::DirectoryEntry& entry)>
        EntitlementRevokedSignal;

      //! Constructs an AccountEntitlementModel.
      /*!
        \param userProfile The user's profile.
        \param account The account whose entitlements are represented by this
               model.
      */
      AccountEntitlementModel(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& account);

      //! Loads the entitlements for the account.
      void Load();

      //! Commits the entitlements to the account.
      void Commit();

      //! Checks if an entitlement has been granted.
      /*!
        \param entry The entry of the entitlement to check.
        \return <code>true</code> iff the entitlement with the specified
                <i>entry</i> was granted.
      */
      bool HasEntitlement(
        const Beam::DirectoryEntry& entry) const;

      //! Grants an entitlement.
      /*!
        \param entry The group entry of the entitlement to grant.
      */
      void Grant(const Beam::DirectoryEntry& entry);

      //! Revokes an entitlement.
      /*!
        \param entry The group entry of the entitlement to revoke.
      */
      void Revoke(const Beam::DirectoryEntry& entry);

      //! Connects a slot to the EntitlementGrantedSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectEntitlementGrantedSignal(
        const EntitlementGrantedSignal::slot_type& slot) const;

      //! Connects a slot to the EntitlementRevokedSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectEntitlementRevokedSignal(
        const EntitlementRevokedSignal::slot_type& slot) const;

    private:
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_account;
      std::unordered_set<Beam::DirectoryEntry>
        m_entitlementsGranted;
      mutable EntitlementGrantedSignal m_entitlementGrantedSignal;
      mutable EntitlementRevokedSignal m_entitlementRevokedSignal;
  };
}

#endif
