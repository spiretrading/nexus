#ifndef SPIRE_DROP_DOWN_BOX_HPP
#define SPIRE_DROP_DOWN_BOX_HPP
#include <any>
#include <QWidget>
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

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
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;
    
      /**
       * Constructs a DropDownBox.
       * @param list_view The list view displayed in the drop down list.
       * @param parent The parent widget.
       */
      explicit DropDownBox(ListView& list_view, QWidget* parent = nullptr);

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
      class DropDownListWrapper;
      friend class DropDownListWrapper;
      mutable SubmitSignal m_submit_signal;
      ListView* m_list_view;
      TextBox* m_text_box;
      Button* m_button;
      DropDownListWrapper* m_drop_down_list;
      boost::optional<int> m_submission;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_current_connection;

      void on_click();
      void on_current(const boost::optional<int>& current);
      void on_submit(const std::any& submission);
      void update_current();
      void update_submission();
  };
}

#endif
