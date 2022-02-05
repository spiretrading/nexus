#ifndef SPIRE_TAB_VIEW_HPP
#define SPIRE_TAB_VIEW_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {
  class TabView : public QWidget {
    public:
      explicit TabView(QWidget* parent = nullptr);
  };
}

#endif
