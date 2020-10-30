#ifndef SPIRE_TRANSITION_TEST_WIDGET_HPP
#define SPIRE_TRANSITION_TEST_WIDGET_HPP
#include <QWidget>
#include "Spire/Ui/TransitionWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a TransitionWidget.
  class TransitionTestWidget : public QWidget {
    public:

      //! Constructs a TransitionTestWidget.
      /*!
        \param parent The parent widget.
      */
      TransitionTestWidget(QWidget* parent = nullptr);

    private:
      TransitionWidget* m_transition_widget;
  };
}

#endif
