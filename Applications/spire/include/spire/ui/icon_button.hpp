#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <QImage>
#include <QLabel>
#include <QString>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a button using an icon.
  class icon_button : public QWidget {
    public:

      //! Signal type for the clicked signal.
      using clicked_signal = signal<void ()>;

      //! Constructs an icon_button.
      /*!
        \param icon The icon to show.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(QImage icon, QWidget* parent = nullptr);

      //! Constructs an icon_button with a default icon and a hover icon.
      /*!
        \param default_icon The icon shown when the button is not hovered.
        \param hover_icon The icon shown when the button is hovered.
        \param parent The parent QWidget to the icon_button.
      */
      icon_button(QImage default_icon, QImage hover_icon,
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

    private:
      mutable clicked_signal m_clicked_signal;
      QLabel* m_label;
      QImage m_default_icon;
      QImage m_hover_icon;
      QString m_default_stylesheet;
      QString m_hover_stylesheet;
  };
}

#endif
