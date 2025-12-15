#ifndef SPIRE_BLOTTERMODEL_HPP
#define SPIRE_BLOTTERMODEL_HPP
#include <optional>
#include <string>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/ActivityLogModel.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/Blotter/OrderLogProperties.hpp"
#include "Spire/Blotter/ProfitAndLossModel.hpp"

namespace Spire {

  /** Stores a single blotter for display in a BlotterWindow. */
  class BlotterModel {
    public:

      /**
       * Constructs a BlotterModel.
       * @param name The name of the blotter.
       * @param executingAccount The account used to execute new Orders.
       * @param isConsolidated Whether this is the <i>executingAccount</i>'s
       *        consolidated blotter.
       * @param userProfile The user's profile.
       * @param taskProperties Used to display the properties of a Task.
       * @param orderLogProperties The Order log's display properties.
       */
      BlotterModel(const std::string& name,
        const Beam::DirectoryEntry& executingAccount,
        bool isConsolidated, Beam::Ref<UserProfile> userProfile,
        const BlotterTaskProperties& taskProperties,
        const OrderLogProperties& orderLogProperties);

      ~BlotterModel();

      /** Returns the name of this blotter. */
      const std::string& GetName() const;

      /** Returns the account used to execute new Orders. */
      const Beam::DirectoryEntry& GetExecutingAccount() const;

      /**
       * Returns <code>true</code> iff this is the executing account's
       * consolidated blotter.
       */
      bool IsConsolidated() const;

      /**
       * Returns <code>true</code> iff this blotter is to be saved upon exit.
       */
      bool IsPersistent() const;

      /** Sets whether this blotter is to be saved upon exit. */
      void SetPersistent(bool value);

      /** Returns the Tasks model. */
      const BlotterTasksModel& GetTasksModel() const;

      /** Returns the Tasks model. */
      BlotterTasksModel& GetTasksModel();

      /** Returns the order log model. */
      const OrderLogModel& GetOrderLogModel() const;

      /** Returns the order log model. */
      OrderLogModel& GetOrderLogModel();

      /** Returns the open positions model. */
      const OpenPositionsModel& GetOpenPositionsModel() const;

      /** Returns the open positions model. */
      OpenPositionsModel& GetOpenPositionsModel();

      /** Returns the profit and loss model. */
      const ProfitAndLossModel& GetProfitAndLossModel() const;

      /** Returns the profit and loss model. */
      ProfitAndLossModel& GetProfitAndLossModel();

      /** Returns the activity log model. */
      const ActivityLogModel& GetActivityLogModel() const;

      /** Returns the activity log model. */
      ActivityLogModel& GetActivityLogModel();

      /** Returns the list of linked Blotters. */
      const std::vector<BlotterModel*>& GetLinkedBlotters() const;

      /**
       * Links this blotter to another.
       * @param blotter The blotter to link to.
       */
      void Link(Beam::Ref<BlotterModel> blotter);

      /**
       * Unlinks this blotter from another.
       * @param blotter The blotter to unlink from.
       */
      void Unlink(BlotterModel& blotter);

    private:
      std::string m_name;
      Beam::DirectoryEntry m_executingAccount;
      bool m_isConsolidated;
      UserProfile* m_userProfile;
      bool m_isPersistent;
      BlotterTasksModel m_tasksModel;
      std::optional<ProfitAndLossModel::PortfolioController>
        m_portfolioController;
      OrderLogModel m_orderLogModel;
      OpenPositionsModel m_openPositionsModel;
      ProfitAndLossModel m_profitAndLossModel;
      ActivityLogModel m_activityLogModel;
      std::unique_ptr<CancelOnFillController> m_cancelOnFillController;
      std::vector<BlotterModel*> m_incomingLinks;
      std::vector<BlotterModel*> m_outgoingLinks;
      EventHandler m_eventHandler;

      BlotterModel(const BlotterModel&) = delete;
      BlotterModel& operator =(const BlotterModel&) = delete;
      void InitializeModels();
      void OnRiskParametersChanged(const Nexus::RiskParameters& riskParameters);
  };
}

#endif
