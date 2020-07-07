#ifndef SPIRE_FONT_SELECTOR_BUTTON_HPP
#define SPIRE_FONT_SELECTOR_BUTTON_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a toggle button for font style options.
  class FontSelectorButton : public QWidget {
    public:

      //! Signal type for click events.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a FontSelectorButton.
      /*!
        \param icon The default icon.
        \param toggle_icon The icon to display when the button is toggled.
        \param hover_icon The icon to display when the button is hovered.
        \param parent The parent widget.
      */
      FontSelectorButton(QImage icon, QImage toggle_icon, QImage hover_icon,
        QWidget* parent = nullptr);

      //! Returns true if the button is toggled.
      bool is_toggled() const;

      //! Sets the toggled status.
      /*!
        \param is_toggled True if the button is toggled, false otherwise.
      */
      void set_toggled(bool is_toggled);

      //! Connects a slot to the button click signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      QImage m_icon;
      QImage m_toggle_icon;
      QImage m_hover_icon;
      bool m_is_toggled;
      Qt::FocusReason m_last_focus_reason;

      void swap_toggle();
  };
}

#endif
