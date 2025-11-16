#ifndef SPIRE_ACCOUNTINFOMODEL_HPP
#define SPIRE_ACCOUNTINFOMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"

namespace Spire {
  class UserProfile;

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
        const Beam::DirectoryEntry& account);

      //! Loads the account's info.
      void Load();

      //! Commits the info.
      void Commit();

      //! Returns the represented account's entry.
      const Beam::DirectoryEntry& GetAccount() const;

      //! Returns the account's identity.
      const Nexus::AccountIdentity& GetIdentity() const;

      //! Returns the account's identity.
      Nexus::AccountIdentity& GetIdentity();

    private:
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_account;
      Nexus::AccountIdentity m_identity;
  };
}

#endif
