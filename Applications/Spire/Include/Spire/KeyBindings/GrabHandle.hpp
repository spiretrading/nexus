#ifndef SPIRE_GRAB_HANDLE_HPP
#define SPIRE_GRAB_HANDLE_HPP
#include <QWidget>
#include "Spire/Ui/HoverObserver.hpp"

namespace Spire {

  /** Represents a grab handle. */
  class GrabHandle : public QWidget {
    public:

      /**
       * Constructs a GrabHandle.
       * @param parent The parent widget.
       */
      explicit GrabHandle(QWidget* parent = nullptr);

    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      HoverObserver m_hover_observer;

      void on_hover(HoverObserver::State state);
  };
}

#endif
