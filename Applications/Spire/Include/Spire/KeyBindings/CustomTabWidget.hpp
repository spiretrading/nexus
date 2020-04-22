#ifndef SPIRE_CUSTOM_TAB_WIDGET_HPP
#define SPIRE_CUSTOM_TAB_WIDGET_HPP
#include <QTabWidget>

namespace Spire {

  class CustomTabWidget : public QTabWidget {
    public:

      CustomTabWidget(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
