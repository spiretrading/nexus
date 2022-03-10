#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include <memory>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** The type of list model used for orders displayed in the blotter. */
  using OrderListModel = ListModel<Nexus::OrderExecutionService::Order*>;

  /**
   * Defines the interface representing a blotter, used to keep track of trading
   * activity.
   */
  class BlotterModel {
    public:
      virtual ~BlotterModel() = default;

      /** Returns the name of this blotter. */
      virtual std::shared_ptr<TextModel> get_name() = 0;

      /** Returns the blotter's positions. */
      virtual std::shared_ptr<BlotterPositionsModel> get_positions() = 0;

      /** Returns the blotter's orders. */
      virtual std::shared_ptr<OrderListModel> get_orders() = 0;

      /** Returns the blotter's status. */
      virtual std::shared_ptr<BlotterStatusModel> get_status() = 0;

    protected:

      /** Constructs a BlotterModel. */
      BlotterModel() = default;

    private:
      BlotterModel(const BlotterModel&) = delete;
      BlotterModel& operator =(const BlotterModel&) = delete;
  };
}

#endif
