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

      //! Constructs an IconButton with a default icon and a hover icon.
      /*!
        \param icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param parent The parent QWidget to the IconButton.
      */
      IconButton(QImage icon, QImage hover_icon, QWidget* parent = nullptr);

      //! Constructs an IconButton with a default icon, hover icon, and
      //!  blur icon.
      /*!
        \param icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param blur_icon The icon shown when the window lacks focus.
        \param parent The parent QWidget to the IconButton.
      */
      IconButton(QImage icon, QImage hover_icon, QImage blur_icon,
        QWidget* parent = nullptr);

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

      //! Returns the icon displayed.
      const QImage& get_icon() const;

      //! Sets the icon to display.
      void set_icon(QImage icon);

      //! Sets the icons to display.
      void set_icon(QImage icon, QImage hover_icon);

      //! Sets the icons to display.
      void set_icon(QImage icon, QImage hover_icon, QImage blur_icon);

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
      QImage m_hover_icon;
      QImage m_blur_icon;
      QColor m_default_background_color;
      QColor m_hover_background_color;

      const QImage& get_current_icon() const;
      bool is_last_focus_reason_tab() const;
  };
}

#endif
