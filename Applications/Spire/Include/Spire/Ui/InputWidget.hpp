#ifndef SPIRE_INPUT_WIDGET_HPP
#define SPIRE_INPUT_WIDGET_HPP
#include <QLineEdit>

namespace Spire {

  class InputWidget : public QLineEdit {
    public:

      explicit InputWidget(QWidget* parent = nullptr);
  };
}

#endif
