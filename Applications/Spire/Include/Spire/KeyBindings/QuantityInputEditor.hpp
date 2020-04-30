#ifndef SPIRE_QUANTITY_INPUT_EDITOR_HPP
#define SPIRE_QUANTITY_INPUT_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  //! Represents a line input for editing Quantities.
  class QuantityInputEditor : public QLineEdit {
    public:

      //! Constructs a QuantityInputEditor.
      /*
        \param parent The parent widget.
      */
      explicit QuantityInputEditor(QWidget* parent = nullptr);

    protected:
      void keyReleaseEvent(QKeyEvent* event) override;
  };
}

#endif
