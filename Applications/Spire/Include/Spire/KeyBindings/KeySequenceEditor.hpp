#ifndef SPIRE_KEY_SEQUENCE_EDITOR_HPP
#define SPIRE_KEY_SEQUENCE_EDITOR_HPP
#include <QLineEdit>

namespace Spire {

  class KeySequenceEditor : public QLineEdit {
    public:

      KeySequenceEditor(QWidget* parent = nullptr);
  };
}

#endif
