#ifndef SPIRE_BUTTON_HPP
#define SPIRE_BUTTON_HPP
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a button built upon a box.
  class Button : public Box {
    public:

      //! Signals that the button is clicked.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a Button.
      /*!
        \param text_box The text box that is displayed inside the button.
        \param parent The parent widget to the Box.
      */
      explicit Button(TextBox* text_box, QWidget* parent = nullptr);

      //! Gets the text of the button.
      const QString& get_text() const;

      //! Sets the text of the button.
      void set_text(const QString& label);

      //! Gets styles of the text.
      const TextBox::Styles& get_text_styles() const;

      //! Sets styles to the text.
      void set_text_styles(const TextBox::Styles& styles);

      //! Connects a slot to the click signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      TextBox* m_text_box;
  };

  Button* make_button(const QString& label, QWidget* parent = nullptr);
}

#endif
