#ifndef SPIRE_TASK_KEYS_PAGE_HPP
#define SPIRE_TASK_KEYS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a widget for the task key bindings. */
  class TaskKeysPage : public QWidget {
    public:

      /**
       * Constructs a TaskKeysPage.
       * @param key_bindings The task key bindings to display.
       * @param destinations The destination database to use.
       * @param markets The market database to use.
       * @param parent The parent widget.
       */
      TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
        Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
        QWidget* parent = nullptr);

      /** Returns the key bindings being displayed. */
      const std::shared_ptr<KeyBindingsModel>& get_key_bindings() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      TableView* m_table_view;
      Button* m_duplicate_button;
      Button* m_delete_button;
      LineInputForm* m_new_task_form;
      TableItem* m_added_region_item;
      bool m_is_row_added;
      boost::signals2::scoped_connection m_selection_connection;
      boost::signals2::scoped_connection m_table_operation_connection;

      void update_button_state();
      void on_new_task_action();
      void on_duplicate_task_action();
      void on_delete_task_action();
      void on_new_task_submission(const QString& name);
      void on_row_selection(const ListModel<int>::Operation& operation);
      void on_table_operation(const TableModel::Operation& operation);
  };
}

#endif
