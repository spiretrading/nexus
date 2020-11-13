#ifndef SPIRE_ICON_BUTTON_TEST_WIDGET_HPP
#define SPIRE_ICON_BUTTON_TEST_WIDGET_HPP
#include <QWidget>

namespace Spire {

  //! Represents a widget for testing an IconButton.
  class IconButtonTestWidget : public QWidget {
    public:

      //! Constructs an IconButtonTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit IconButtonTestWidget(QWidget* parent = nullptr);
  };
}

#endif
