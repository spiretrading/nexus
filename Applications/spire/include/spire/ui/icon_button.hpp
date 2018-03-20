#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <boost/signals2/connection.hpp>
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#include <QFocusEvent>
#include <QImage>
#include <QLabel>
#include <QRect>
#include <QString>
#include <QWidget>

namespace spire {

  class icon_button : public QWidget {
    public:

      //! Signal type for the clicked signal.
      using clicked_signal = boost::signals2::signal_type<void (),
        boost::signals2::keywords::mutex_type<
        boost::signals2::dummy_mutex>>::type;

      //! Constructs an icon_button with a default icon and a hover icon. The
      //  icons are scaled to cover the whole button.
      /*!
        \param default_icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param width The width of the icon_button.
        \param height The height of the icon_button.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(const QString& default_icon, const QString& hover_icon,
        int width, int height, QWidget* parent = nullptr);

      //! Constructs an icon_button with a default icon and a hover icon. The
      //  icons are scaled and drawn according to the draw rect.
      /*!
        \param default_icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param width The width of the icon_button.
        \param height The height of the icon_button.
        \param draw_rect The position and size of the image within the
                         icon_button.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(const QString& default_icon, const QString& hover_icon,
        int width, int height, const QRect& draw_rect,
        QWidget* parent = nullptr);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

      //! Sets whether the button can have its m_clicked_signal activated or not.
      /*!
        \param clickable Whether the button is clickable (true) or not (false).
      */
      void set_clickable(bool clickable);

      //! Switches the button's icons.
      void swap_icons();

      //! Sets the button's icon.
      /*!
        \param is_default True to set the icon to default, false to set it to
                          the hover icon.
      */
      void set_icon(bool is_default);

      //! Sets if the button can be focused by the keyboard.
      /*!
        \param focusable True if the button takes keyboard focus, false
                         otherwise.
      */
      void set_focusable(bool focusable);

    protected:
      void enterEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable clicked_signal m_clicked_signal;
      QLabel* m_label;
      QImage m_default_icon;
      QImage m_hover_icon;
      bool m_clickable;
      bool m_focusable;
      bool m_is_default;
  };
}

#endif
