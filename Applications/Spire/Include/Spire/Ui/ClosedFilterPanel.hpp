#ifndef SPIRE_CLOSED_FILTER_PANEL_HPP
#define SPIRE_CLOSED_FILTER_PANEL_HPP
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/FilterPanel.hpp"

namespace Spire {
  class ListView;

  /** Displays a FilterPanel over a closed list of values. */
  class ClosedFilterPanel : public QWidget {
    public:

      /**
       * Signals the list of values selected by the user.
       * @param submission The list of values selected by the user.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AnyListModel>& submission)>;

      /**
       * Constructs a ClosedFilterPanel.
       * @param table A TableModel whose first column is a selectable value,
       *        and whose second column is a boolean value indicating whether
       *        the value in the first column is currently included.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      ClosedFilterPanel(
        std::shared_ptr<TableModel> table, QString title, QWidget& parent);

      /** Returns the table of values and whether they are selected. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TableModel> m_table;
      std::shared_ptr<ArrayListModel<std::any>> m_submission;
      FilterPanel* m_filter_panel;
      ListView* m_list_view;
      boost::signals2::scoped_connection m_table_connection;

      void on_list_model_operation(const AnyListModel::Operation& operation);
      void on_table_model_operation(const TableModel::Operation& operation);
      void on_reset();
  };
}

#endif
