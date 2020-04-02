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

KeySequenceEditor::KeySequenceEditor(const std::set<Qt::Key>& valid_keys,
    QWidget* parent)
    : QLineEdit(parent),
      m_valid_keys(valid_keys),
      m_font("Roboto") {
  m_font.setPixelSize(scale_height(12));
  m_font.setStyle(QFont::StyleItalic);
}

connection KeySequenceEditor::connect_key_sequence_signal(
    const KeySequenceSignal::slot_type& slot) const {
  return m_key_sequence_signal.connect(slot);
}

void KeySequenceEditor::keyPressEvent(QKeyEvent* event) {
  if(!event->isAutoRepeat()) {
    m_entered_keys.push_back(static_cast<Qt::Key>(event->key()));
  }
  //event->accept();
}

void KeySequenceEditor::keyReleaseEvent(QKeyEvent* event) {
  if(!m_entered_keys.empty() && is_valid(m_entered_keys)) {
    m_key_sequence_signal(make_key_sequence(m_entered_keys));
  }
  m_entered_keys.clear();
}

void KeySequenceEditor::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, 0, width(), height(), Qt::white);
  painter.setFont(m_font);
  painter.setPen(Qt::black);
  painter.drawText(0, scale_height(16), tr("Enter Keys"));
}

bool KeySequenceEditor::is_valid(const std::vector<Qt::Key>& keys) {
  if(keys.size() > 2) {
    return false;
  }
  return true;
}
