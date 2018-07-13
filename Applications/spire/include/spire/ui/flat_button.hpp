#ifndef SPIRE_FLAT_BUTTON_HPP
#define SPIRE_FLAT_BUTTON_HPP
#include <QLabel>
#include <QWidget>
#include "spire/spire/spire.hpp"

namespace spire {

  //! Displays a flat button with no click animation.
  class flat_button : public QWidget {
    public:

      //! Stores the colors of the styled elements.
      struct style {

        //! The main background color.
        QColor m_background_color;

        //! The border color. Set to transparent to disable the border.
        QColor m_border_color;

        //! The text color.
        QColor m_text_color;
      };

      //! Signals that the button was clicked.
      using clicked_signal = signal<void ()>;

      //! Constructs a flat_button without text.
      /*!
        \param parent The parent widget to the flat_button.
      */
      flat_button(QWidget* parent = nullptr);

      //! Constructs the flat_button.
      /*!
        \param label The label text.
        \param parent The parent widget to the flat_button.
      */
      flat_button(const QString& label, QWidget* parent = nullptr);

      //! Sets the flat_button's text.
      /*!
        \param text The text to set.
      */
      void set_text(const QString& text);

      //! Gets the default button style.
      const style& get_style() const;

      //! Sets the default button style.
      void set_style(const style& default_style);
      
      //! Gets the style of the button when hovered.
      const style& get_hover_style() const;

      //! Sets the style of the button when hovered.
      void set_hover_style(const style& hover_style);

      //! Gets the style of the button when focused with the keyboard.
      const style& get_focus_style() const;

      //! Sets the style of the button when focused with the keyboard.
      void set_focus_style(const style& focus_style);

      //! Gets the style of the button when it's disabled.
      const style& get_disabled_style() const;

      //! Sets the style of the button when it's disabled.
      void set_disabled_style(const style& disabled_style);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QLabel* m_label;
      mutable clicked_signal m_clicked_signal;
      bool m_clickable;
      style m_default_style;
      style m_hover_style;
      style m_focus_style;
      style m_disabled_style;
      Qt::FocusReason m_last_focus_reason;

      void disable_button();
      void enable_button();
      QString get_stylesheet_properties(const style& s);
      void set_disabled_stylesheet();
      void set_focus_stylesheet();
      void set_hover_stylesheet();
      void on_style_updated();
  };
}

#endif
