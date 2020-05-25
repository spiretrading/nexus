#ifndef SPIRE_CUSTOM_TAB_WIDGET_HPP
#define SPIRE_CUSTOM_TAB_WIDGET_HPP
#include <QTabWidget>

namespace Spire {

  //! Represents a Spire-style tab widget.
  class CustomTabWidget : public QTabWidget {
    public:

      //! Constructs a CustomTabWidget.
      /*
        \param parent The parent widget.
      */
      explicit CustomTabWidget(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
