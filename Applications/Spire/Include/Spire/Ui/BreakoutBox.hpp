#ifndef SPIRE_BREAKOUT_BOX_HPP
#define SPIRE_BREAKOUT_BOX_HPP
#include <QWidget>

namespace Spire {

  /**
   * Takes a body and allows the body to grow beyond the layout while staying
   * within its containing window.
   */
  class BreakoutBox : public QWidget {
    public:

      /**
       * Constructs a BreakoutBox.
       * @param body The component that can extend beyond its layout.
       * @param source The component that used to own the body.
       */
      BreakoutBox(QWidget& body, QWidget& source);

      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      QWidget* m_body;
      QWidget* m_source;

      void update_maximum_size();
  };
}

#endif
