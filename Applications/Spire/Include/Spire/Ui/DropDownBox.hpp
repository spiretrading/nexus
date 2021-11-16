#ifndef SPIRE_DROP_DOWN_BOX_HPP
#define SPIRE_DROP_DOWN_BOX_HPP
#include <any>
#include <QWidget>
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects a widget displaying a pop-up, ie. the DropDownBox's list. */
  using PopUp = StateSelector<void, struct PopUpSelectorTag>;
}

  /**
   * Represents a widget which allows the user to choose one value from
   * a drop down list.
   */
  class DropDownBox : public QWidget {
    public:

      /**
       * The type of model representing the index of the current value.
       */
      using CurrentModel = ListView::CurrentModel;

      /**
       * The type of model representing the index of the selected value.
       */
      using SelectionModel = ListView::SelectionModel;

      /**
       * The type of function used to build a QWidget representing a value.
       */
      using ViewBuilder = ListView::ViewBuilder;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /**
       * Constructs a DropDownBox using default local models and a default view
       * builder.
       * @param list_model The model of values to display.
       * @param parent The parent widget.
       */
      explicit DropDownBox(std::shared_ptr<ListModel> list_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list_model The model of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<ListModel> list_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list_model The model of values to display.
       * @param current_model The current value's model.
       * @param selection_model The selection value's model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<ListModel> list_model,
        std::shared_ptr<CurrentModel> current_model,
        std::shared_ptr<SelectionModel> selection_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /** Returns the list of selectable values. */
      const std::shared_ptr<ListModel>& get_list_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection_model() const;

      /** Returns <code>true</code> iff this DropDownBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DropDownBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      ListView* m_list_view;
      TextBox* m_text_box;
      Button* m_button;
      DropDownList* m_drop_down_list;
      boost::optional<int> m_submission;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_current_connection;

      void on_click();
      void on_current(const boost::optional<int>& current);
      void on_submit(const std::any& submission);
      void revert_current();
      void submit();
  };
}

#endif
