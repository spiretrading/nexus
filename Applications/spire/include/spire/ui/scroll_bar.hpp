#ifndef SPIRE_SCROLL_BAR
#define SPIRE_SCROLL_BAR
#include <QScrollBar>
#include "spire/ui/ui.hpp"

namespace spire {

  //! A custom Spire scroll bar.
  class scroll_bar : public QScrollBar {
    public:

      //! Constructs a scroll_bar.
      /*!
        \param pos The initial position of the scroll_bar relative to it's
                   parent.
        \param parent The parent widget.
      */
      scroll_bar(Qt::Orientation orientation, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
  };
}

#endif SPIRE_SCROLL_BAR
