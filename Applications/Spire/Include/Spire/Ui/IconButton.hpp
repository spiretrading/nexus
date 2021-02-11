#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <QAbstractButton>
#include <QImage>
#include <QString>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon.
  class IconButton : public QAbstractButton {
    public:

      //! Represents the style options for an IconButton.
      struct Style {

        //! The icon's color when the parent window lacks focus.
        QColor m_blur_color;

        //! The icon's color when the button is checked.
        QColor m_checked_color;

        //! The icon's color when the button is checked and the window lacks
        //! focus.
        QColor m_checked_blur_color;

        //! The icon's color when the button is checked and hovered.
        QColor m_checked_hovered_color;

        //! The icon's default color.
        QColor m_default_color;

        //! The icon's disabled color.
        QColor m_disabled_color;

        //! The icon's hover color.
        QColor m_hover_color;

        //! The button's default background color.
        QColor m_default_background_color;

        //! The button's hover background color.
        QColor m_hover_background_color;

        //! Constructs a Style with the default style.
        Style();
      };

      //! Constructs an IconButton with the default Style.
      /*!
        \param icon The icon to show.
        \param parent The parent widget.
      */
      explicit IconButton(QImage icon, QWidget* parent = nullptr);

      //! Constructs an IconButton with a custom style.
      /*!
        \param icon The icon to show.
        \param style The button's style.
        \param parent The parent widget.
      */
      IconButton(QImage icon, Style style, QWidget* parent = nullptr);

      //! Returns the IconButton's Style.
      const Style& get_style() const;

      //! Sets the IconButton's Style.
      /*!
        \param style The current Style.
      */
      void set_style(const Style& style);

    protected:
      bool event(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      QSize sizeHint() const override;

    private:
      QImage m_icon;
      Style m_style;
      Tooltip* m_tooltip;

      const QColor& get_background_color() const;
      const QColor& get_current_icon_color() const;
  };
}

#endif
