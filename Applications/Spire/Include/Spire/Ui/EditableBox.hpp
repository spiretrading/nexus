#ifndef SPIRE_EDITABLE_BOX_HPP
#define SPIRE_EDITABLE_BOX_HPP
#include <chrono>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/MouseObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Encapsulates an AnyInputBox which can change editing status. */
  class EditableBox : public QWidget {
    public:

      /**
       * Signals a change to the read-only state.
       * @param read_only Whether this EditableBox is read only.
       */
      using ReadOnlySignal = Signal<void(bool read_only)>;

      /**
       * The type of function used to test whether a given key sequence triggers
       * an edit state.
       * @param key The key sequence used to test.
       * @return <code>true</code> iff the key sequence passes the test.
       */
      using EditTrigger = std::function<bool (const QKeySequence& key)>;

      /**
       * The default edit trigger can trigger the edit state when
       * the key is alphanumeric.
       */
      static bool default_edit_trigger(const QKeySequence& key);

      /**
       * Constructs an EditableBox with a default edit trigger.
       * @param input_box The AnyInputBox to encapsulate.
       * @param parent The parent widget.
       */
      explicit EditableBox(AnyInputBox& input_box, QWidget* parent = nullptr);

      /**
       * Constructs an EditableBox.
       * @param input_box The AnyInputBox to encapsulate.
       * @param trigger The edit trigger for the user input.
       * @param parent The parent widget.
       */
      EditableBox(
        AnyInputBox& input_box, EditTrigger trigger, QWidget* parent = nullptr);

      /** Returns the AnyInputBox. */
      const AnyInputBox& get_input_box() const;

      /** Returns the AnyInputBox. */
      AnyInputBox& get_input_box();

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /** Sets whether the box is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the ReadOnlySignal. */
      boost::signals2::connection connect_read_only_signal(
        const ReadOnlySignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      mutable ReadOnlySignal m_read_only_signal;
      AnyInputBox* m_input_box;
      EditTrigger m_edit_trigger;
      FocusObserver m_focus_observer;
      MouseObserver m_mouse_observer;
      QWidget* m_focus_proxy;
      boost::optional<std::chrono::steady_clock::time_point> m_focus_time;
      bool m_is_submit_connected;

      void install_focus_proxy_event_filter();
      void select_all_text();
      bool on_click(QWidget& target, QMouseEvent& event);
      void on_focus(FocusObserver::State state);
      void on_submit(const AnyRef& submission);
  };
}

#endif
