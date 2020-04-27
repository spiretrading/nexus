#ifndef SPIRE_NAME_INPUT_EDITOR_HPP
#define SPIRE_NAME_INPUT_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  class NameInputEditor : public QLineEdit {
    public:

      explicit NameInputEditor(QWidget* parent = nullptr);

    protected:
      void keyReleaseEvent(QKeyEvent* event) override;
  };
}

#endif
