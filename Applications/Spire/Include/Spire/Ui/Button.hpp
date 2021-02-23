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

      //! Gets the default text style.
      const TextBox::Style& get_text_style() const;

      //! Sets the default text style.
      void set_text_style(const TextBox::Style& style);

      //! Gets the text style of the button when hovered.
      const TextBox::Style& get_text_hover_style() const;

      //! Sets the text style of the button when hovered.
      void set_text_hover_style(const TextBox::Style& style);

      //! Gets the text style of the button when focused.
      const TextBox::Style& get_text_focus_style() const;

      //! Sets the text style of the button when focused.
      void set_text_focus_style(const TextBox::Style& style);

      //! Gets the text style of the button when it's disabled.
      const TextBox::Style& get_text_disabled_style() const;

      //! Sets the text style of the button when disabled.
      void set_text_disabled_style(const TextBox::Style& style);

      //! Connects a slot to the click signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void process_style(const Style& style) override;
      void process_hover_style(const Style& hover_style) override;
      void process_focus_style(const Style& focus_style) override;
      void process_disabled_style(const Style& disabled_style) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      TextBox* m_text_box;
  };

  Button* make_button(const QString& label, QWidget* parent = nullptr);
}

#endif
