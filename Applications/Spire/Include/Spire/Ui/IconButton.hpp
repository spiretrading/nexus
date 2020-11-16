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

      //! Signal type for the clicked signal.
      using ClickedSignal = Signal<void ()>;

      //! Constructs an IconButton.
      /*!
        \param icon The icon to show.
        \param parent The parent QWidget to the IconButton.
      */
      explicit IconButton(QImage icon, QWidget* parent = nullptr);

      //! Sets the icon to display.
      /*!
        \param icon The icon image.
      */
      void set_icon(const QImage& icon);

      //! Sets the icon's color when the window lacks focus.
      /*!
        \param color The blur color.
      */
      void set_blur_color(const QColor& color);

      //! Sets the icon's default color.
      /*!
        \param color The default color.
      */
      void set_default_color(const QColor& color);

      //! Sets the icon's hover color.
      /*!
        \param color The hover color.
      */
      void set_hover_color(const QColor& color);

      //! Sets the default background color for the button.
      /*!
        \param color The default background color.
      */
      void set_default_background_color(const QColor& color);

      //! Sets the background color for when the button is hovered.
      /*!
        \param color The hover background color.
      */
      void set_hover_background_color(const QColor& color);

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
      QColor m_default_color;
      QColor m_hover_color;
      QColor m_disabled_color;
      QColor m_blur_color;
      QColor m_default_background_color;
      QColor m_hover_background_color;

      const QColor& get_current_icon_color() const;
      bool is_last_focus_reason_tab() const;
  };
}

#endif
