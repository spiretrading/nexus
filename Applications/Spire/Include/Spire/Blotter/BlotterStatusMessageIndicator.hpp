#ifndef SPIRE_BLOTTER_STATUS_MESSAGE_INDICATOR_HPP
#define SPIRE_BLOTTER_STATUS_MESSAGE_INDICATOR_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays an icon indicating whether there are new execution report
   * messages to view within a button that will popup a window listing unread
   * messages.
   */
  class BlotterStatusMessageIndicator : public QWidget {
    public:

      /**
       * Constructs a BlotterStatusMessageIndicator.
       * @param parent The parent widget.
       */
      explicit BlotterStatusMessageIndicator(QWidget* parent = nullptr);

    private:
      ToggleButton* m_button;
  };
}

#endif
