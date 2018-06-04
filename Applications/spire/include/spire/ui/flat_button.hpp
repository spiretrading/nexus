#ifndef SPIRE_FLAT_BUTTON_HPP
#define SPIRE_FLAT_BUTTON_HPP
#include <QLabel>
#include <QWidget>
#include "spire/spire/spire.hpp"

namespace spire {

  //! Displays a flat button with no click animation.
  class flat_button : public QWidget {
    public:

      //! Signals that the button was clicked.
      using clicked_signal = signal<void ()>;

      //! Constructs a flat_button without text.
      /*!
        \param parent The parent widget to the flat_button.
      */
      flat_button(QWidget* parent = nullptr);

      //! Constructs the flat_button.
      /*!
        \param label The label text.
        \param parent The parent widget to the flat_button.
      */
      flat_button(const QString& label, QWidget* parent = nullptr);

      //! Sets the flat_button's text.
      /*!
        \param text The text to set.
      */
      void set_text(const QString& text);

      //! Sets the flat_button's stylesheet. Note that these styles are
      //! wrapped in the appropriate CSS selectors, so only the properties
      //! need to be specified.
      /*!
        \param default_style The text, background, and border style when
               the flat_button isn't hovered or focused.
        \param hover_style The style when the flat_button is hovered.
        \param focused_style The style when the flat_button is focused.
        \param disabled_style The style when the flat_button is disabled.
      */
      void set_stylesheet(const QString& default_style,
        const QString& hover_style, const QString& focused_style,
        const QString& disabled_style);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QLabel* m_label;
      mutable clicked_signal m_clicked_signal;
      bool m_clickable;
      QString m_default_style;
      QString m_hover_style;
      QString m_focused_style;
      QString m_disabled_style;
      Qt::FocusReason m_last_focus_reason;

      void disable_button();
      void enable_button();
      void set_disabled_stylesheet();
      void set_focused_stylesheet();
      void set_hover_stylesheet();
  };
}

#endif
