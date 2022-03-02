#ifndef SPIRE_BLOTTER_STATUS_MODEL_HPP
#define SPIRE_BLOTTER_STATUS_MODEL_HPP
#include <memory>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/MoneyBox.hpp"

namespace Spire {

  /** Defines the interface representing a blotter's status and summary. */
  class BlotterStatusModel {
    public:
      virtual ~BlotterStatusModel() = default;

      /** Returns the account's available buying power. */
      virtual std::shared_ptr<MoneyModel> get_buying_power() = 0;

      /** Returns the account's net loss. */
      virtual std::shared_ptr<MoneyModel> get_net_loss() = 0;

      /** Returns the blotter's total profit and loss. */
      virtual std::shared_ptr<MoneyModel> get_total_profit_and_loss() = 0;

      /** Returns the blotter's unrealized profit and loss. */
      virtual std::shared_ptr<MoneyModel> get_unrealized_profit_and_loss() = 0;

      /** Returns the blotter's realized profit and loss. */
      virtual std::shared_ptr<MoneyModel> get_realized_profit_and_loss() = 0;

      /** Returns the blotter's fees. */
      virtual std::shared_ptr<MoneyModel> get_fees() = 0;

      /** Returns the blotter's cost basis. */
      virtual std::shared_ptr<MoneyModel> get_cost_basis() = 0;

    protected:

      /** Constructs a BlotterStatusModel. */
      BlotterStatusModel() = default;

    private:
      BlotterStatusModel(const BlotterStatusModel&) = delete;
      BlotterStatusModel& operator =(const BlotterStatusModel&) = delete;
  };
}

#endif
