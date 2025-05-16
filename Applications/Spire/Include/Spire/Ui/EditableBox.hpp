#ifndef SPIRE_EDITABLE_BOX_HPP
#define SPIRE_EDITABLE_BOX_HPP
#include <chrono>
#include <functional>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/MouseObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct model_type {
    using type = typename
      std::remove_cvref_t<decltype(*(std::declval<T>().get_current()))>::Type;
  };

  template<typename T>
  using model_type_t = typename model_type<T>::type;
}

  /** Encapsulates an AnyInputBox which can change editing status. */
  class EditableBox : public QWidget {
    public:

      /**
       * Signals a change to the read-only state.
       * @param read_only Whether this EditableBox is read only.
       */
      using ReadOnlySignal = Signal<void(bool read_only)>;

      /** Function used to reset the input box. */
      using DefaultValue = std::function<std::any ()>;

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

      /** The default function used to reset an input box. */
      template<typename T>
      static std::any make_default_value();

      /**
       * Constructs an EditableBox with a default edit trigger and using a
       * type's default constructor to reset the input box.
       * @param input_box The AnyInputBox to encapsulate.
       * @param parent The parent widget.
       */
      template<typename T> requires(!std::is_same_v<T, AnyInputBox>)
      explicit EditableBox(T& input_box, QWidget* parent = nullptr);

      /**
       * Constructs an EditableBox with a default edit trigger and using a
       * type's default constructor to reset the input box.
       * @param input_box The AnyInputBox to encapsulate.
       * @param trigger The edit trigger for the user input.
       * @param parent The parent widget.
       */
      template<typename T> requires(!std::is_same_v<T, AnyInputBox>)
      EditableBox(T& input_box, EditTrigger trigger, QWidget* parent = nullptr);

      /**
       * Constructs an EditableBox with a default edit trigger.
       * @param input_box The AnyInputBox to encapsulate.
       * @param make_default_value The function used to reset the
       *        <i>input_box</i>.
       * @param parent The parent widget.
       */
      EditableBox(AnyInputBox& input_box, DefaultValue make_default_value,
        QWidget* parent = nullptr);

      /**
       * Constructs an EditableBox.
       * @param input_box The AnyInputBox to encapsulate.
       * @param make_default_value The function used to reset the
       *        <i>input_box</i>.
       * @param trigger The edit trigger for the user input.
       * @param parent The parent widget.
       */
      EditableBox(AnyInputBox& input_box, DefaultValue make_default_value,
        EditTrigger trigger, QWidget* parent = nullptr);

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
      void keyPressEvent(QKeyEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      mutable ReadOnlySignal m_read_only_signal;
      AnyInputBox* m_input_box;
      std::function<std::any ()> m_make_default_value;
      EditTrigger m_edit_trigger;
      FocusObserver m_focus_observer;
      MouseObserver m_mouse_observer;
      QWidget* m_focus_proxy;
      boost::optional<std::chrono::steady_clock::time_point> m_focus_time;
      bool m_is_submit_connected;


      bool is_input_box_enabled() const;
      void select_all_text();
      bool on_click(QWidget& target, QMouseEvent& event);
      void on_focus(FocusObserver::State state);
      void on_submit(const AnyRef& submission);
  };

  template<typename T>
  std::any EditableBox::make_default_value() {
    return T();
  }

  template<typename T> requires(!std::is_same_v<T, AnyInputBox>)
  EditableBox::EditableBox(T& input_box, QWidget* parent)
    : EditableBox(*new AnyInputBox(input_box),
        &make_default_value<Details::model_type_t<T>>, parent) {}

  template<typename T> requires(!std::is_same_v<T, AnyInputBox>)
  EditableBox::EditableBox(
    T& input_box, EditTrigger edit_trigger, QWidget* parent)
    : EditableBox(*new AnyInputBox(input_box),
        &make_default_value<Details::model_type_t<T>>, std::move(edit_trigger),
        parent) {}
}

#endif
