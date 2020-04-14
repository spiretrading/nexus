#ifndef SPIRE_INPUT_FIELD_EDITOR_HPP
#define SPIRE_INPUT_FIELD_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  class InputFieldEditor : public QLineEdit {
    public:

      InputFieldEditor(QWidget* parent = nullptr);
  };
}

#endif
