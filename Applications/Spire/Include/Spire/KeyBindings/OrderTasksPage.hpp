#ifndef SPIRE_ORDER_TASKS_PAGE_HPP
#define SPIRE_ORDER_TASKS_PAGE_HPP
#include <QWidget>
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskTableModel.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /** Displays a page configuring key bindings for order tasks. */
  class OrderTasksPage : public QWidget {
    public:

      using OrderTask = OrderTaskTableModel::OrderTask;

      /**
       * Constructs a OrderTasksPage.
       * @param region_query_model The model used to query region matches.
       * @param model The list model of OrderTask.
       * @param destination_database The destination database.
       * @param market_database The market database.
       * @param parent The parent widget.
       */
      OrderTasksPage(std::shared_ptr<ComboBox::QueryModel> region_query_model,
        std::shared_ptr<ListModel<OrderTask>> model,
        const Nexus::DestinationDatabase& destination_database,
        const Nexus::MarketDatabase& market_database,
        QWidget* parent = nullptr);

      /** Returns the model used to query region matches. */
      const std::shared_ptr<ComboBox::QueryModel>&
        get_region_query_model() const;

      /** Returns the list model. */
      const std::shared_ptr<ListModel<OrderTask>>& get_model() const;

    private:
      std::shared_ptr<ComboBox::QueryModel> m_region_query_model;
      std::shared_ptr<ListModel<OrderTask>> m_list_model;
      std::shared_ptr<OrderTaskTableModel> m_table;
  };
}

#endif
