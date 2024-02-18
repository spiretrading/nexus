#ifndef SPIRE_EDITABLE_BOX_HPP
#define SPIRE_EDITABLE_BOX_HPP
#include <QWidget>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Encapsulates an AnyInputBox which can change editing status. */
  class EditableBox : public QWidget {
    public:

      /** Tests whether a given string triggers an edit state. */
      static bool is_edit_trigger(const QString& text);

      /** Signals that this box has started editing. */
      using StartEditSignal = Signal<void()>;

      /** Signals that this box has quit editing. */
      using EndEditSignal = Signal<void ()>;

      /**
       * Constructs a EditableBox.
       * @param input_box The AnyInputBox to encapsulate.
       * @param parent The parent widget.
       */
      explicit EditableBox(AnyInputBox& input_box, QWidget* parent = nullptr);

      /** Returns the AnyInputBox. */
      const AnyInputBox& get_input_box() const;

      /** Returns the AnyInputBox. */
      AnyInputBox& get_input_box();

      /** Returns <code>true</code> iff this box is being edited. */
      bool is_editing() const;

      /**
       * Sets the edit state.
       * @param is_editing True iff this box starts editing.
       */
      void set_editing(bool is_editing);

      /** Connects a slot to the StartEditSignal. */
      boost::signals2::connection connect_start_edit_signal(
        const StartEditSignal::slot_type& slot) const;

      /** Connects a slot to the EndEditSignal. */
      boost::signals2::connection connect_end_edit_signal(
        const EndEditSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      mutable StartEditSignal m_start_edit_signal;
      mutable EndEditSignal m_end_edit_signal;
      AnyInputBox* m_input_box;
      FocusObserver m_focus_observer;
      QWidget* m_focus_proxy;
      boost::signals2::scoped_connection m_submit_connection;

      void install_focus_proxy_event_filter();
      void select_all_text();
      void on_focus(FocusObserver::State state);
      void on_submit(const AnyRef& submission);
  };
}

#endif
