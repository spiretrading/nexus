#ifndef SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Composes a blotter from supplied models. */
  class CompositeBlotterModel : public BlotterModel {
    public:

      /**
       * Constructs a CompositeBlotterModel.
       * @param account The executing account.
       * @param is_consolidated Whether the blotter consolidates its account.
       * @param name The blotter's name model.
       * @param is_active Whether the blotter is active.
       * @param is_pinned Whether the blotter is pinned for saving on exit.
       * @param tasks The blotter's order execution task entries.
       * @param monitors The monitor definitions for those tasks.
       * @param orders The blotter's orders.
       * @param portfolio The blotter's portfolio.
       */
      CompositeBlotterModel(Beam::DirectoryEntry account, bool is_consolidated,
        std::shared_ptr<TextModel> name,
        std::shared_ptr<BooleanModel> is_active,
        std::shared_ptr<BooleanModel> is_pinned,
        std::shared_ptr<TasksModel> tasks,
        std::shared_ptr<TaskMonitorsModel> monitors,
        std::shared_ptr<OrdersModel> orders,
        std::shared_ptr<PortfolioModel> portfolio) noexcept;

      const Beam::DirectoryEntry& get_account() const override;
      bool is_consolidated() const override;
      const std::shared_ptr<TextModel>& get_name() const override;
      const std::shared_ptr<BooleanModel>& is_active() const override;
      const std::shared_ptr<BooleanModel>& is_pinned() const override;
      const std::shared_ptr<TasksModel>& get_tasks() const override;
      const std::shared_ptr<TaskMonitorsModel>&
        get_task_monitors() const override;
      const std::shared_ptr<OrdersModel>& get_orders() const override;
      const std::shared_ptr<PortfolioModel>& get_portfolio() const override;

    private:
      Beam::DirectoryEntry m_account;
      bool m_is_consolidated;
      std::shared_ptr<TextModel> m_name;
      std::shared_ptr<BooleanModel> m_is_active;
      std::shared_ptr<BooleanModel> m_is_pinned;
      std::shared_ptr<TasksModel> m_tasks;
      std::shared_ptr<TaskMonitorsModel> m_monitors;
      std::shared_ptr<OrdersModel> m_orders;
      std::shared_ptr<PortfolioModel> m_portfolio;
  };
}

#endif
