#ifndef SPIRE_BREAKOUT_BOX_HPP
#define SPIRE_BREAKOUT_BOX_HPP
#include <QWidget>
#include "Spire/Ui/GlobalPositionObserver.hpp"

namespace Spire {

  /**
   * Takes a body and allows the body to grow beyond the layout while staying
   * within its containing window.
   */
  class BreakoutBox : public QWidget {
    public:

      /**
       * Constructs a BreakoutBox.
       * @param body The component that can extend beyond the layout.
       * @param parent The parent widget.
       */
      explicit BreakoutBox(QWidget& body, QWidget* parent = nullptr);

      /** Returns the body. */
      QWidget& get_body();

      /** Returns <code>true</code> iff the body has escaped from the layout. */
      bool is_broken_out() const;

      /** Breaks the body out of the layout. */
      void breakout();

      /** Restores the body within the layout. */
      void restore();

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      GlobalPositionObserver m_position_observer;

      void adjust_size();
      void update_size_constraints();
      void on_position(const QPoint& position);
  };
}

#endif
