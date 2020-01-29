#ifndef SPIRE_SCALAR_WIDGET_HPP
#define SPIRE_SCALAR_WIDGET_HPP
#include <QWidget>
#include "Spire/Spire/Scalar.hpp"

namespace Spire {

  //! Provides a common widget with an interface for getting and setting a
  //! value on another widget that displays a scalar value.
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
      ScalarWidget(W* widget,
        boost::signals2::connection (W::* connector)(Signal<T>),
        std::function<void (const T&)> setter);

      //! Constructs a ScalarWidget from a widget that uses a Qt signal for
      //! signaling updates to its value.
      /*
        \param widget The widget to display and manage getting/setting values
                      for.
        \param connector Callback for when the widget's value has changed.
        \param setter Method to call to set the value that the widget displays.
      */
      template<typename W, typename T>
      ScalarWidget(W* widget, void (W::* callback)(T),
        std::function<void (const T&)> setter);

      //! Sets the widget's value by calling its setter method.
      void set_value(Scalar value);

    private:
      mutable ChangeSignal m_change_signal;
      boost::signals2::scoped_connection m_callback_connection;
      std::function<void (Scalar)> m_setter;

      void on_widget_value_changed(Scalar value);
      void set_layout(QWidget* widget);
  };
}

#endif
