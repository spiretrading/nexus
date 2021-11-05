#ifndef SPIRE_CLOSED_FILTER_PANEL_HPP
#define SPIRE_CLOSED_FILTER_PANEL_HPP
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a FilterPanel over a closed list of values. */
  class ClosedFilterPanel : public QWidget {
    public:

      /**
       * Signals the list of values selected by the user.
       * @param submission The list of values selected by the user.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<ListModel>& submission)>;

      /**
       * Constructs a ClosedFilterPanel.
       * @param model A TableModel whose first column is a selectable value,
       *        and whose second column is a boolean value indicating whether
       *        the value in the first column is currently included.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      ClosedFilterPanel(std::shared_ptr<TableModel> model, QString title,
        QWidget& parent);

      /** Returns the table of values and whether they are selected. */
      const std::shared_ptr<TableModel>& get_model() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      struct ModelItem {
        std::shared_ptr<BooleanModel> m_model;
        boost::signals2::scoped_connection m_connection;
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TableModel> m_model;
      std::shared_ptr<ArrayListModel> m_submission;
      std::vector<std::unique_ptr<ModelItem>> m_model_items;
      FilterPanel* m_filter_panel;
      ListView* m_list_view;
      boost::signals2::scoped_connection m_model_connection;

      void on_current(const std::any& value, bool is_checked);
      void on_list_model_operation(const ListModel::Operation& operation);
      void on_reset();
  };
}

#endif
