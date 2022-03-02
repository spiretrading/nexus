#ifndef SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#define SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's execution reports. */
  class BlotterExecutionsView : public QWidget {
    public:

      /**
       * Constructs a BlotterExecutionsView.
       * @param parent The parent widget.
       */
      explicit BlotterExecutionsView(QWidget* parent = nullptr);

    private:
      Box* m_box;
  };
}

#endif
