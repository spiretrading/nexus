#ifndef SPIRE_FLAT_BUTTON_HPP
#define SPIRE_FLAT_BUTTON_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a flat button with no click animation.
  class FlatButton : public QWidget {
    public:

      //! Stores the colors of the styled elements.
      struct Style {

        //! The main background color.
        QColor m_background_color;

        //! The border color. Set to transparent to disable the border.
        QColor m_border_color;

        //! The text color.
        QColor m_text_color;
      };

      //! Signals that the button was clicked.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a FlatButton without text.
      /*!
        \param parent The parent widget to the FlatButton.
      */
      explicit FlatButton(QWidget* parent = nullptr);

      //! Constructs the FlatButton.
      /*!
        \param label The label text.
        \param parent The parent widget to the FlatButton.
      */
      explicit FlatButton(const QString& label, QWidget* parent = nullptr);

      //! Sets the FlatButton's label.
      /*!
        \param label The label to set.
      */
      void set_label(const QString& label);

      //! Gets the default button style.
      const Style& get_style() const;

      //! Sets the default button style.
      void set_style(const Style& default_style);
      
      //! Gets the style of the button when hovered.
      const Style& get_hover_style() const;

      //! Sets the style of the button when hovered.
      void set_hover_style(const Style& hover_style);

      //! Gets the style of the button when focused with the keyboard.
      const Style& get_focus_style() const;

      //! Sets the style of the button when focused with the keyboard.
      void set_focus_style(const Style& focus_style);

      //! Gets the style of the button when it's disabled.
      const Style& get_disabled_style() const;

      //! Sets the style of the button when it's disabled.
      void set_disabled_style(const Style& disabled_style);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      QLabel* m_label;
      bool m_clickable;
      Style m_default_style;
      Style m_hover_style;
      Style m_focus_style;
      Style m_disabled_style;
      Qt::FocusReason m_last_focus_reason;

      void disable_button();
      void enable_button();
      QString get_stylesheet_properties(const Style& s);
      void set_disabled_stylesheet();
      void set_focus_stylesheet();
      void set_hover_stylesheet();
      void on_style_updated();
  };

  //! Constructs a FlatButton with the default style.
  /*!
    \param label The button's label.
    \param parent The parent widget.
  */
  FlatButton* make_flat_button(const QString& label, QWidget* parent = nullptr);
}

#endif
