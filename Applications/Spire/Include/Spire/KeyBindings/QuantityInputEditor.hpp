#ifndef SPIRE_QUANTITY_INPUT_EDITOR_HPP
#define SPIRE_QUANTITY_INPUT_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  class QuantityInputEditor : public QLineEdit {
    public:

      explicit QuantityInputEditor(QWidget* parent = nullptr);

    protected:
      void keyReleaseEvent(QKeyEvent* event) override;
  };
}

#endif
