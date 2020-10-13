#ifndef SPIRE_FLAT_BUTTON_TEST_WIDGET_HPP
#define SPIRE_FLAT_BUTTON_TEST_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Displays a widget for testing FlatButtons.
  class FlatButtonTestWidget : public QWidget {
    public:

      //! Constructs a FlatButtonTestWidget.
      /*
        \param parent The parent widget.
      */
      explicit FlatButtonTestWidget(QWidget* parent = nullptr);
  };
}

#endif
