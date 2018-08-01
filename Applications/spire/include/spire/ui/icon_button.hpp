#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <QAbstractButton>
#include <QImage>
#include <QString>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a button using an icon.
  class icon_button : public QAbstractButton {
    public:

      //! Signal type for the clicked signal.
      using clicked_signal = Signal<void ()>;

      //! Constructs an icon_button.
      /*!
        \param icon The icon to show.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(QImage icon, QWidget* parent = nullptr);

      //! Constructs an icon_button with a default icon and a hover icon.
      /*!
        \param icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(QImage icon, QImage hover_icon, QWidget* parent = nullptr);

      //! Constructs an icon_button with a default icon, hover icon, and
      //!  blur icon.
      /*!
        \param icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param blur_icon The icon shown when the window lacks focus.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(QImage icon, QImage hover_icon, QImage blur_icon,
        QWidget* parent = nullptr);

      //! Sets the default stylesheet for the button.
      /*!
        \param stylesheet The default stylesheet.
      */
      void set_default_style(const QString& stylesheet);

      //! Sets the stylesheet for when the button is hovered.
      /*!
        \param stylesheet The hover stylesheet.
      */
      void set_hover_style(const QString& stylesheet);

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
        const clicked_signal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      bool event(QEvent* event) override;

    private:
      enum class state {
        NORMAL,
        HOVERED,
        BLURRED,
        HOVER_BLURRED
      };
      mutable clicked_signal m_clicked_signal;
      state m_state;
      QImage m_icon;
      QImage m_hover_icon;
      QImage m_blur_icon;
      QString m_default_stylesheet;
      QString m_hover_stylesheet;

      void show_normal();
      void show_hovered();
      void show_blurred();
      void show_hover_blurred();
  };
}

#endif
