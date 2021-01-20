#ifndef SPIRE_FLAT_BUTTON_HPP
#define SPIRE_FLAT_BUTTON_HPP
#include <QPushButton>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a flat button with no click animation.
  class FlatButton : public QPushButton {
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

      //! Sets the button style.
      /*!
        \param default_style The default button style.
        \param hover_style The style of the button when hovered.
        \param focus_style The style of the button when focused with the keyboard.
        \param disabled_style The style of the button when it's disabled.
      */
      void set_style(const Style& default_style, const Style& hover_style,
        const Style& focus_style, const Style& disabled_style);

      //! Gets the default button style.
      const Style& get_default_style() const;

      //! Sets the default button style.
      void set_default_style(const Style& default_style);

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

      //! Sets the properties of the button font.
      /*!
        \param pixel_size The pixel size of the font.
        \param weight The weight of the font.
      */
      void set_font_properties(int pixel_size, int weight);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      QSize sizeHint() const override;

    private:
      Style m_default_style;
      Style m_hover_style;
      Style m_focus_style;
      Style m_disabled_style;

      void update_style();
  };
}

#endif
