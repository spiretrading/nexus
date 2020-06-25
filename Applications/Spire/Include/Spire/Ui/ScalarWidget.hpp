#ifndef SPIRE_SCALAR_WIDGET_HPP
#define SPIRE_SCALAR_WIDGET_HPP
#include <QWidget>
#include "Spire/Spire/Scalar.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Wraps a QWidget used to edit a scalar type T into a QWidget that edits a
  //! Scalar.
  class ScalarWidget : public QWidget {
    public:

      //! Signals that the widget's value has changed.
      using ChangeSignal = Signal<void (Scalar)>;

      //! Constructs a ScalarWidget from a widget that uses a boost signal for
      //! signaling updates to its value.
      /*
        \param widget The widget to display and manage getting/setting values
                      for.
        \param connector Callback for when the widget's value has changed.
        \param setter Method to call to set the value that the widget displays.
      */
      template<typename W, typename T>
      ScalarWidget(W* widget, boost::signals2::connection (W::* connector)(
        const typename Signal<void (T)>::slot_type&) const,
        void (W::* setter)(T));

      //! Returns the widget's current value.
      Scalar get_value() const;

      //! Sets the widget's value by calling its setter method.
      void set_value(Scalar value);

      //! Signals that the widget's value has changed.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    private:
      mutable ChangeSignal m_change_signal;
      boost::signals2::scoped_connection m_callback_connection;
      std::function<void (Scalar)> m_setter;
      Scalar m_value;

      void on_widget_value_changed(Scalar value);
      void set_layout(QWidget* widget);
  };

  template<typename W, typename T>
  ScalarWidget::ScalarWidget(W* widget,
      boost::signals2::connection (W::* connector)(
      const typename Signal<void (T)>::slot_type&) const,
      void (W::* setter)(T))
      : m_setter([=] (Scalar s) {
          (widget->*setter)(static_cast<T>(s));
        }) {
    m_callback_connection = (widget->*connector)([=] (T value) {
      on_widget_value_changed(static_cast<Scalar>(value));
    });
    set_layout(widget);
  }
}

#endif
