#ifndef SPIRE_ACCOUNTINFOMODEL_HPP
#define SPIRE_ACCOUNTINFOMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class AccountInfoModel
      \brief Stores an account's info.
   */
  class AccountInfoModel {
    public:

      //! Constructs an AccountInfoModel.
      /*!
        \param userProfile The user's profile.
        \param account The account whose entitlements are represented by this
               model.
      */
      AccountInfoModel(Beam::Ref<UserProfile> userProfile,
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Loads the account's info.
      void Load();

      //! Commits the info.
      void Commit();

      //! Returns the represented account's entry.
      const Beam::ServiceLocator::DirectoryEntry& GetAccount() const;

      //! Returns the account's identity.
      const Nexus::AdministrationService::AccountIdentity& GetIdentity() const;

      //! Returns the account's identity.
      Nexus::AdministrationService::AccountIdentity& GetIdentity();

    private:
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_account;
      Nexus::AdministrationService::AccountIdentity m_identity;
  };
}

#endif
