#ifndef SPIRE_TAB_TEST_WIDGET_HPP
#define SPIRE_TAB_TEST_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Represents a widget for testing a TabWidget.
  class TabTestWidget : public QWidget {
    public:

      //! Constructs a TabTestWidget.
      /*!
        \param parent The parent widget.
      */
      TabTestWidget(QWidget* parent = nullptr);
  };
}

#endif
