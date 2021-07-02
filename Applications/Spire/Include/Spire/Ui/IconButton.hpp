#ifndef SPIRE_ICON_BUTTON_HPP
#define SPIRE_ICON_BUTTON_HPP
#include <QAbstractButton>
#include <QImage>
#include <QString>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon.
  class IconButton : public QWidget {
    public:

      /** Signals that the IconButton is clicked. */
      using ClickedSignal = Signal<void ()>;

      //! Constructs an IconButton.
      /*!
        \param icon The icon to show.
        \param parent The parent widget.
      */
      explicit IconButton(Icon* icon, QWidget* parent = nullptr);

      /** Connects a slot to the click signal. */
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      Button* m_button;
      Tooltip* m_tooltip;
  };
}

#endif
