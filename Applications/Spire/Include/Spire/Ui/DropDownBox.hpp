#ifndef SPIRE_DROP_DOWN_BOX_HPP
#define SPIRE_DROP_DOWN_BOX_HPP
#include <any>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a widget which allows the user to choose one value from a list.
   */
  class DropDownBox : public QWidget {
    public:

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal =
        Signal<void (const boost::optional<std::any>& submission)>;
    
      /**
       * Constructs a DropDownBox.
       * @param list_view The list view displayed in the drop down list.
       * @param parent The parent widget.
       */
      explicit DropDownBox(ListView& list_view, QWidget* parent = nullptr);
    
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
      Box* m_input_box;
      Button* m_button;
      DropDownList* m_drop_down_list;
      boost::optional<int> m_submission_index;
      boost::signals2::scoped_connection m_list_view_submit_connection;
      boost::signals2::scoped_connection m_list_view_current_connection;

      void on_click();
      void on_list_view_current(const boost::optional<int>& current);
      void on_list_view_submit(const std::any& submission);
      void on_escape_press();
      void on_panel_close();
      void update_submission();
  };
}

#endif
