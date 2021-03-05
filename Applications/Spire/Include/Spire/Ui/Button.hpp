#ifndef SPIRE_BUTTON_HPP
#define SPIRE_BUTTON_HPP
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a button built upon a box.
  class Button : public Styles::StyledWidget {
    public:

      //! Signals that the button is clicked.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a Button.
      /*!
        \param component The widget is displayed insdie.
        \param parent The parent widget to the Box.
      */
      explicit Button(QWidget* component, QWidget* parent = nullptr);

      //! Connects a slot to the click signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void focusOutEvent(QFocusEvent* event);
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      QWidget* m_component;
      bool m_is_down;
  };

  Button* make_label_button(const QString& label, QWidget* parent = nullptr);
}

#endif
