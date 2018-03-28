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

      //! Sets whether the button can have its m_clicked_signal activated or not.
      /*!
        \param clickable Whether the button is clickable (true) or not (false).
      */
      void set_clickable(bool clickable);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QLabel* m_label;
      mutable clicked_signal m_clicked_signal;
      bool m_clickable;
  };
}

#endif
