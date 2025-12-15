#ifndef SPIRE_RISKMODEL_HPP
#define SPIRE_RISKMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Spire {
  class UserProfile;

  /*! \class RiskModel
      \brief Stores an account's risk parameters.
   */
  class RiskModel {
    public:

      //! Constructs an RiskModel.
      /*!
        \param userProfile The user's profile.
        \param account The account whose risk is represented by this model.
      */
      RiskModel(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& account);

      //! Loads the account's risk model.
      void Load();

      //! Commits the risk model.
      void Commit();

      //! Returns the represented account's entry.
      const Beam::DirectoryEntry& GetAccount() const;

      //! Returns the account's RiskParameters.
      const Nexus::RiskParameters& GetRiskParameters() const;

      //! Returns the account's RiskParameters.
      Nexus::RiskParameters& GetRiskParameters();

    private:
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_account;
      Nexus::RiskParameters m_riskParameters;
  };
}

#endif
