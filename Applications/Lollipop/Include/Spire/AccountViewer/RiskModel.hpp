#ifndef SPIRE_RISKMODEL_HPP
#define SPIRE_RISKMODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

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
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Loads the account's risk model.
      void Load();

      //! Commits the risk model.
      void Commit();

      //! Returns the represented account's entry.
      const Beam::ServiceLocator::DirectoryEntry& GetAccount() const;

      //! Returns the account's RiskParameters.
      const Nexus::RiskService::RiskParameters& GetRiskParameters() const;

      //! Returns the account's RiskParameters.
      Nexus::RiskService::RiskParameters& GetRiskParameters();

    private:
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_account;
      Nexus::RiskService::RiskParameters m_riskParameters;
  };
}

#endif
