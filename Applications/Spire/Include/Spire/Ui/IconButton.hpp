#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <QAbstractButton>
#include <QImage>
#include <QString>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon.
  class IconButton : public QAbstractButton {
    public:

      //! Represents the style options for an IconButton.
      struct Style {

        //! The icon's color when the parent window lacks focus.
        QColor m_blur_color;

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

      //! Signal type for the clicked signal.
      using ClickedSignal = Signal<void ()>;

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

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      Qt::FocusReason m_last_focus_reason;
      QImage m_icon;
      Style m_style;

      const QColor& get_current_icon_color() const;
      bool is_last_focus_reason_tab() const;
  };
}

#endif
