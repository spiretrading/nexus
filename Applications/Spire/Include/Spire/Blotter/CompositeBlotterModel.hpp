#ifndef SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Implements the BlotterModel by composing constituent models together. */
  class CompositeBlotterModel : public BlotterModel {
    public:

      /**
       * Constructs a CompositeBlotterModel.
       * @param name Represents the name of this blotter.
       * @param is_active Whether this blotter is the active blotter.
       * @param is_pinned Whether this blotter is pinned.
       * @param tasks The list of tasks belonging to this blotter.
       * @param task_selection The list of selected tasks.
       * @param valuation The valuation used by this blotter.
       * @param orders Represents this blotter's orders.
       * @param profit_and_loss Represents this blotter's profit and loss.
       * @param status Represents the blotter's status/summary.
       */
      CompositeBlotterModel(std::shared_ptr<TextModel> name,
        std::shared_ptr<BooleanModel> is_active,
        std::shared_ptr<BooleanModel> is_pinned,
        std::shared_ptr<BlotterTaskListModel> tasks,
        std::shared_ptr<ListModel<int>> task_selection,
        std::shared_ptr<ValuationModel> valuation,
        std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
        std::shared_ptr<BlotterStatusModel> status);

      std::shared_ptr<TextModel> get_name() override;

      std::shared_ptr<BooleanModel> is_active() override;

      std::shared_ptr<BooleanModel> is_pinned() override;

      std::shared_ptr<BlotterTaskListModel> get_tasks() override;

      std::shared_ptr<ListModel<int>> get_task_selection() const override;

      std::shared_ptr<ValuationModel> get_valuation() override;

      std::shared_ptr<BlotterProfitAndLossModel> get_profit_and_loss()
        override;

      std::shared_ptr<BlotterStatusModel> get_status() override;

    private:
      std::shared_ptr<TextModel> m_name;
      std::shared_ptr<BooleanModel> m_is_active;
      std::shared_ptr<BooleanModel> m_is_pinned;
      std::shared_ptr<BlotterTaskListModel> m_tasks;
      std::shared_ptr<ListModel<int>> m_task_selection;
      std::shared_ptr<ValuationModel> m_valuation;
      std::shared_ptr<BlotterProfitAndLossModel> m_profit_and_loss;
      std::shared_ptr<BlotterStatusModel> m_status;
  };

  /** Makes a CompositeBlotterModel using local models as its constituents. */
  std::shared_ptr<CompositeBlotterModel> make_local_blotter_model();

  /**
   * Makes a CompositeBlotterModel whose positions, orders, profit and loss,
   * and status is derived from its tasks model.
   * @param name Represents the name of this blotter.
   * @param is_active Whether this blotter is the active blotter.
   * @param is_pinned Whether this blotter is pinned.
   * @param tasks The list of tasks belonging to this blotter.
   */
  std::shared_ptr<CompositeBlotterModel> make_derived_blotter_model(
    std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned,
    std::shared_ptr<BlotterTaskListModel> tasks);
}

#endif
