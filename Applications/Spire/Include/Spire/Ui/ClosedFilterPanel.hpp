#ifndef SPIRE_CLOSED_FILTER_PANEL_HPP
#define SPIRE_CLOSED_FILTER_PANEL_HPP
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** Displays a FilterPanel over a closed list of values. */
  class ClosedFilterPanel : public QWidget {
    public:

      /**
       * Signals a list of selected values to filter out.
       * @param submission A list of selected values.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AnyListModel>& submission)>;

      /**
       * Constructs a ClosedFilterPanel.
       * @param table A TableModel whose first column is a selectable value,
       *        and whose second column is a boolean value indicating whether
       *        the value in the first column is currently included.
       * @param parent The parent widget.
       */
      explicit ClosedFilterPanel(std::shared_ptr<TableModel> table,
        QWidget* parent = nullptr);

      /** Returns the table of values and whether they are selected. */
      const std::shared_ptr<TableModel>& get_table() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TableModel> m_table;
      std::shared_ptr<ArrayListModel<std::any>> m_submission;
      boost::signals2::scoped_connection m_table_connection;

      void on_list_model_operation(const AnyListModel::Operation& operation);
      void on_table_model_operation(const TableModel::Operation& operation);
      void on_reset();
  };
}

#endif
