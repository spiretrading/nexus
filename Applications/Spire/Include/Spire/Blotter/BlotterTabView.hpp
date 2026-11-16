#ifndef SPIRE_BLOTTER_TAB_VIEW_HPP
#define SPIRE_BLOTTER_TAB_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a blotter's secondary views. */
  class BlotterTabView : public QWidget {
    public:

      /**
       * Constructs a BlotterTabView with empty tab bodies.
       * @param parent The parent widget.
       */
      explicit BlotterTabView(QWidget* parent = nullptr);
  };
}

#endif
