#ifndef SPIRE_TASK_KEYS_PAGE_HPP
#define SPIRE_TASK_KEYS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/SecurityBox.hpp"

namespace Spire {
  class Button;
  class TableView;

  /** Implements a widget for the task key bindings. */
  class TaskKeysPage : public QWidget {
    public:

      /**
       * Constructs a TaskKeysPage.
       * @param key_bindings The task key bindings to display.
       * @param securities The set of securities to use.
       * @param additional_tags Defines all available additional tags.
       * @param parent The parent widget.
       */
      TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<SecurityInfoQueryModel> securities,
        AdditionalTagDatabase additional_tags, QWidget* parent = nullptr);

      /** Returns the key bindings being displayed. */
      const std::shared_ptr<KeyBindingsModel>& get_key_bindings() const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      TableView* m_table_view;
      Button* m_duplicate_button;
      Button* m_delete_button;
      Button* m_reset_button;

      void update_button_state();
      void on_new_task_action();
      void on_duplicate_task_action();
      void on_delete_task_action();
      void on_reset();
      void on_new_task_submission(const QString& name);
      void on_row_selection(const ListModel<int>::Operation& operation);
  };
}

#endif
