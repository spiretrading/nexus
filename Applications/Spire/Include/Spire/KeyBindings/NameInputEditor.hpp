#ifndef SPIRE_NAME_INPUT_EDITOR_HPP
#define SPIRE_NAME_INPUT_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  //! Represents a line edit for editing a key binding's name value.
  class NameInputEditor : public QLineEdit {
    public:

      //! Constructs a NameInputEditor.
      /*
        \param parent The parent widget.
      */
      explicit NameInputEditor(QWidget* parent = nullptr);

    protected:
      void keyReleaseEvent(QKeyEvent* event) override;
  };
}

#endif
