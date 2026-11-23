#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Manages a blotter's order execution tasks, orders, and positions. */
  class BlotterModel {
    public:
      virtual ~BlotterModel() = default;

      /** Returns the blotter's name. */
      virtual const std::shared_ptr<TextModel>& get_name() const = 0;

      /** Returns whether the blotter is active. */
      virtual const std::shared_ptr<BooleanModel>& is_active() const = 0;

      /** Returns whether the blotter is pinned for saving on exit. */
      virtual const std::shared_ptr<BooleanModel>& is_pinned() const = 0;

      /** Returns the blotter's order execution task entries. */
      virtual const std::shared_ptr<TasksModel>& get_tasks() const = 0;

      /** Returns the monitor definitions in task column order. */
      virtual const std::shared_ptr<TaskMonitorsModel>&
        get_task_monitors() const = 0;

      /** Returns the blotter's orders. */
      virtual const std::shared_ptr<OrdersModel>& get_orders() const = 0;

      /** Returns the blotter's portfolio. */
      virtual const std::shared_ptr<PortfolioModel>& get_portfolio() const = 0;
  };
}

#endif
