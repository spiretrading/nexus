#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a box container with a spcific styling.
  class Box : public QWidget {
    public:

      //! Represents the styling of the box.
      struct Style {

        //! The size of the box in pixels or as a percentage of its container.
        boost::optional<boost::variant<QSize, QSizeF>> m_size;

        //! The width of the top, right, bottom and left border.
        boost::optional<QMargins> m_borders;

        //! The width of the top, right, bottom and left padding.
        boost::optional<QMargins> m_paddings;

        //! The background color.
        QColor m_background_color;

        //! The border color.
        QColor m_border_color;
      };

      //! Constructs a Box widget.
      /*!
        \param parent The parent to the Box widget.
      */
      explicit Box(QWidget* parent = nullptr);

      //! Gets the default box style.
      const Style& get_style() const;

      //! Sets the default box style.
      void set_style(const Style& style);

      //! Gets the style of the box widget when hovered.
      const Style& get_hover_style() const;

      //! Sets the style of the box widget when hovered.
      void set_hover_style(const Style& hover_style);

      //! Gets the style of the box widget when focused with the keyboard.
      const Style& get_focus_style() const;

      //! Sets the style of the box widget when focused with the keyboard.
      void set_focus_style(const Style& focus_style);

      //! Gets the style of the box widget when it's disabled.
      const Style& get_disabled_style() const;

      //! Sets the style of the box widget when it's disabled.
      void set_disabled_style(const Style& disabled_style);

    protected:
      void changeEvent(QEvent* event) override;
      bool event(QEvent* event) override;
      QSize sizeHint() const override;

    private:
      Style m_style;
      Style m_hover_style;
      Style m_focus_style;
      Style m_disabled_style;
      bool m_is_hover;

      QSize get_size(const Style& style) const;
      void resize_box(const Style& style);
      void update_box();
      void change_padding(const Style& style);
      void update_style();
  };
}

#endif
