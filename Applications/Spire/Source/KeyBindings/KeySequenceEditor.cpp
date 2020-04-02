#include "Spire/KeyBindings/KeySequenceEditor.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto make_key_sequence(const std::vector<Qt::Key>& keys) {
    if(keys.size() == 1) {
      return QKeySequence(keys[0]);
    }
    return QKeySequence(keys[0], keys[1]);
  }
}

KeySequenceEditor::KeySequenceEditor(const QKeySequence& sequence,
    const std::set<Qt::Key>& valid_keys, QWidget* parent)
    : QLineEdit(parent),
      m_key_sequence(sequence),
      m_valid_keys(valid_keys) {
  setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;
    font-style: italic;
  )").arg(scale_height(12)));
}

const QKeySequence& KeySequenceEditor::get_key_sequence() const {
  return m_key_sequence;
}

void KeySequenceEditor::keyPressEvent(QKeyEvent* event) {
  if(!event->isAutoRepeat()) {
    m_entered_keys.push_back(static_cast<Qt::Key>(event->key()));
  }
  event->accept();
}

void KeySequenceEditor::keyReleaseEvent(QKeyEvent* event) {
  if(is_valid(m_entered_keys)) {
    m_key_sequence = make_key_sequence(m_entered_keys);
  }
  m_entered_keys.clear();
  emit editingFinished();
}

bool KeySequenceEditor::is_valid(const std::vector<Qt::Key>& keys) {
  if(keys.size() > 2) {
    return false;
  }
  return true;
}
