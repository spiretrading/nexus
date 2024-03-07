#ifndef SPIRE_ORDER_TASKS_KEY_BINDINGS_FORM_HPP
#define SPIRE_ORDER_TASKS_KEY_BINDINGS_FORM_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsToTableModel.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /** Displays a form configuring key bindings for order tasks. */
  class OrderTasksKeyBindingsForm : public QWidget {
    public:

      /**
       * Constructs a OrderTasksKeyBindingsForm.
       * @param region_query_model The model used to query region matches.
       * @param order_task_arguments The list of order task arguments.
       * @param destinations The destination database to use.
       * @param markets The market database to use.
       * @param parent The parent widget.
       */
      OrderTasksKeyBindingsForm(
        std::shared_ptr<ComboBox::QueryModel> region_query_model,
        std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments,
        Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
        QWidget* parent = nullptr);

      /** Returns the model used to query region matches. */
      const std::shared_ptr<ComboBox::QueryModel>&
        get_region_query_model() const;

      /** Returns the list of order task arguments. */
      const std::shared_ptr<OrderTaskArgumentsListModel>&
        get_order_task_arguments() const;

    private:
      using Column = OrderTaskArgumentsToTableModel::Column;
      std::shared_ptr<ComboBox::QueryModel> m_region_query_model;
      std::shared_ptr<OrderTaskArgumentsListModel> m_order_task_arguments;
      Nexus::DestinationDatabase m_destinations;
      Nexus::MarketDatabase m_markets;

      QWidget* make_cell(const std::shared_ptr<TableModel>& table,
        int row, int column);
      EditableBox* make_editor(const std::shared_ptr<TableModel>& table,
        int row, int column);
      EditableBox* make_empty_editor(const std::shared_ptr<TableModel>& table,
        int row, int column);
      void on_submit(AnyInputBox* input_box, Column column,
        const AnyRef& submission);
  };
}

#endif
