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
    const std::set<Qt::Key>& valid_first_keys,
    const std::set<Qt::Key>& valid_second_keys, QWidget* parent)
    : QLineEdit(parent),
      m_key_sequence(sequence),
      m_valid_first_keys(valid_first_keys),
      m_valid_second_keys(valid_second_keys),
      m_font("Roboto") {
  m_font.setPixelSize(scale_height(12));
  m_font.setStyle(QFont::StyleItalic);
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

void KeySequenceEditor::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, scale_height(1), width(), height(), Qt::white);
  painter.setFont(m_font);
  painter.setPen(Qt::black);
  painter.drawText(0, scale_height(16), tr("Enter Keys"));
}

bool KeySequenceEditor::is_valid(const std::vector<Qt::Key>& keys) {
  if(keys.empty() || keys.size() > 2) {
    return false;
  }
  if(!m_valid_first_keys.empty()) {
    if(m_valid_first_keys.find(keys[0]) == m_valid_first_keys.end()) {
      return false;
    }
  }
  if(!m_valid_second_keys.empty()) {
    if(keys.size() == 2) {
      if(m_valid_second_keys.find(keys[1]) == m_valid_second_keys.end()) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}
