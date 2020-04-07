#include "Spire/KeyBindings/KeySequenceEditor.hpp"
#include <QKeyEvent>
#include <QPainter>
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
    const std::vector<ValidKeySequence>& valid_sequences, QWidget* parent)
    : QLineEdit(parent),
      m_key_sequence(sequence),
      m_valid_sequences(valid_sequences),
      m_font("Roboto") {
  m_font.setPixelSize(scale_height(12));
  m_font.setStyle(QFont::StyleItalic);
}

void KeySequenceEditor::add_key(Qt::Key key) {
  m_entered_keys.push_back(key);
}

const QKeySequence& KeySequenceEditor::get_key_sequence() const {
  return m_key_sequence;
}

void KeySequenceEditor::keyPressEvent(QKeyEvent* event) {
  event->accept();
  if(!event->isAutoRepeat()) {
    if(event->key() == Qt::Key_Delete) {
      m_key_sequence = QKeySequence();
      commit_sequence();
    }
    m_entered_keys.push_back(static_cast<Qt::Key>(event->key()));
  }
}

void KeySequenceEditor::keyReleaseEvent(QKeyEvent* event) {
  if(is_valid(m_entered_keys)) {
    m_key_sequence = make_key_sequence(m_entered_keys);
  }
  commit_sequence();
}

void KeySequenceEditor::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, scale_height(1), width(), height(), Qt::white);
  painter.setFont(m_font);
  painter.setPen(Qt::black);
  painter.drawText(0, scale_height(16), tr("Enter Keys"));
}

void KeySequenceEditor::commit_sequence() {
  m_entered_keys.clear();
  emit editingFinished();
}

bool KeySequenceEditor::is_valid(const std::vector<Qt::Key>& keys) const {
  if(keys.empty() || keys.size() > 2) {
    return false;
  }
  for(auto& sequence : m_valid_sequences) {
    for(auto i = std::size_t(0); i < sequence.size(); ++i) {
      if(sequence[i].find(m_entered_keys[i]) == sequence[i].end()) {
        break;
      }
      if(i == sequence.size() - 1) {
        return true;
      }
    }
  }
  return false;
}
