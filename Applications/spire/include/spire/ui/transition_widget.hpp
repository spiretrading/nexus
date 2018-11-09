#ifndef SPIRE_TRANSITION_WIDGET_HPP
#define SPIRE_TRANSITION_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Displays a transition animation on top of another widget.
  class TransitionWidget : public QWidget {
    public:

      //! Constructs a transition widget.
      /*
        \param parent The parent widget, and the widget that the transition
                      animation will be displayed on top of.
      */
      explicit TransitionWidget(QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      void align();
  };
}

#endif
