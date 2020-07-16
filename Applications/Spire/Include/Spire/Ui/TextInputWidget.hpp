#ifndef SPIRE_TEXT_INPUT_WIDGET_HPP
#define SPIRE_TEXT_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class TextInputWidget : public QLineEdit {
    public:

      TextInputWidget(QWidget* parent = nullptr);
  };
}

#endif
