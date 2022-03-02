#ifndef SPIRE_COMPOSITE_BLOTTER_STATUS_MODEL_HPP
#define SPIRE_COMPOSITE_BLOTTER_STATUS_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterStatusModel.hpp"

namespace Spire {

  /**
   * Implements the BlotterStatusModel by composing constituent models together.
   */
  class CompositeBlotterStatusModel : public BlotterStatusModel {
    public:

      /**
       * Constructs a CompositeBlotterStatusModel.
       * @param buying_power Represents the buying power.
       * @param net_loss Represents the net loss.
       * @param total_profit_and_loss Represents the total profit and loss.
       * @param unrealized_profit_and_loss Represents the unrealized profit and
       *        loss.
       * @param realized_profit_and_loss Represents the realized profit and
       *        loss.
       * @param fees Represents the fees.
       * @param cost_basis Represents the cost basis.
       */
      CompositeBlotterStatusModel(std::shared_ptr<MoneyModel> buying_power,
        std::shared_ptr<MoneyModel> net_loss,
        std::shared_ptr<MoneyModel> total_profit_and_loss,
        std::shared_ptr<MoneyModel> unrealized_profit_and_loss,
        std::shared_ptr<MoneyModel> realized_profit_and_loss,
        std::shared_ptr<MoneyModel> fees,
        std::shared_ptr<MoneyModel> cost_basis);

      std::shared_ptr<MoneyModel> get_buying_power() override;

      std::shared_ptr<MoneyModel> get_net_loss() override;

      std::shared_ptr<MoneyModel> get_total_profit_and_loss() override;

      std::shared_ptr<MoneyModel> get_unrealized_profit_and_loss() override;

      std::shared_ptr<MoneyModel> get_realized_profit_and_loss() override;

      std::shared_ptr<MoneyModel> get_fees() override;

      std::shared_ptr<MoneyModel> get_cost_basis() override;

    private:
      std::shared_ptr<MoneyModel> m_buying_power;
      std::shared_ptr<MoneyModel> m_net_loss;
      std::shared_ptr<MoneyModel> m_total_profit_and_loss;
      std::shared_ptr<MoneyModel> m_unrealized_profit_and_loss;
      std::shared_ptr<MoneyModel> m_realized_profit_and_loss;
      std::shared_ptr<MoneyModel> m_fees;
      std::shared_ptr<MoneyModel> m_cost_basis;
  };

  /**
   * Makes a CompositeBlotterStatusModel where each constituent is represented
   * by a local model.
   */
  std::shared_ptr<CompositeBlotterStatusModel>
    make_local_blotter_status_model();
}

#endif
