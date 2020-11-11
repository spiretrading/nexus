#include "Spire/Ui/KeySequenceInputField.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/KeySequenceValidationModel.hpp"

using namespace Spire;

namespace {
  const auto& TEXT_PADDING() {
    static auto padding = scale_width(5);
    return padding;
  }

  auto get_key_text(Qt::Key key) {
    switch(key) {
      case Qt::Key_Shift:
        return QObject::tr("Shift");
      case Qt::Key_Alt:
        return QObject::tr("Alt");
      case Qt::Key_Control:
        return QObject::tr("Ctrl");
      case Qt::Key_unknown:
        return QObject::tr("NaK");
      default:
        return QKeySequence(key).toString();
    }
  }

  auto make_key_sequence(const std::vector<Qt::Key>& keys) {
    switch(keys.size()) {
      case 1:
        return QKeySequence(keys[0]);
      case 2:
        return QKeySequence(keys[0], keys[1]);
      case 3:
        return QKeySequence(keys[0], keys[1], keys[2]);
      case 4:
        return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
      default:
        return QKeySequence();
    }
  }
}

KeySequenceInputField::KeySequenceInputField(
    std::shared_ptr<KeySequenceValidationModel> model, QWidget* parent)
    : QLineEdit(parent),
      m_model(std::move(model)),
      m_state(State::DEFAULT),
      m_font("Roboto"),
      m_is_last_key_event_release(false) {
  setReadOnly(true);
  m_font.setPixelSize(scale_height(12));
  m_font.setStyle(QFont::StyleItalic);
}

void KeySequenceInputField::add_key(Qt::Key key) {
  m_entered_keys.push_back(key);
}

const QKeySequence& KeySequenceInputField::get_key_sequence() const {
  return m_key_sequence;
}

void KeySequenceInputField::set_key_sequence(const QKeySequence& sequence) {
  if(m_model->is_valid(sequence)) {
    m_key_sequence = sequence;
    update();
  }
}

void KeySequenceInputField::focusOutEvent(QFocusEvent* event) {
  m_is_last_key_event_release = false;
  m_state = State::DEFAULT;
  m_entered_keys.clear();
  QLineEdit::focusOutEvent(event);
}

void KeySequenceInputField::keyPressEvent(QKeyEvent* event) {
  m_is_last_key_event_release = false;
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_state == State::DEFAULT) {
      m_state = State::EDIT;
    } else {
      m_state = State::DEFAULT;
      m_entered_keys.clear();
    }
    update();
    return;
  }
  if(event->key() == Qt::Key_Delete) {
    m_state = State::DEFAULT;
    commit_sequence({});
    return;
  }
  if(!event->isAutoRepeat()) {
    m_entered_keys.push_back(static_cast<Qt::Key>(event->key()));
  }
  m_state = State::EDIT;
  update();
}

void KeySequenceInputField::keyReleaseEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    return;
  }
  if(event->key() == Qt::Key_Delete) {
    m_state = State::DEFAULT;
    commit_sequence({});
  }
  if(m_entered_keys.empty() && (event->key() == Qt::Key_Tab ||
      event->key() == Qt::Key_Backtab)) {
    return;
  }
  if(!m_is_last_key_event_release) {
    auto key_sequence = make_key_sequence(m_entered_keys);
    if(m_model->is_valid(key_sequence)) {
      commit_sequence(key_sequence);
    }
  }
  m_is_last_key_event_release = true;
}

void KeySequenceInputField::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_entered_keys.clear();
    m_state = State::EDIT;
    update();
  }
}

void KeySequenceInputField::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, scale_height(1), width(), height(), Qt::white);
  if(hasFocus() || underMouse()) {
    painter.setPen("#4B23A0");
    painter.drawRect(0, 0, width() - 1, height() - 1);
  } else {
    painter.setPen("#C8C8C8");
    painter.drawRect(0, 0, width() - 1, height() - 1);
  }
  if(m_state == State::DEFAULT) {
    if(!m_key_sequence.isEmpty() && m_key_sequence != Qt::Key_unknown) {
      auto pos = rect().bottomLeft();
      pos.setX(pos.x() + scale_width(8));
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      font.setWeight(55);
      painter.setFont(font);
      auto metrics = QFontMetrics(font);
      for(auto i = 0; i < m_key_sequence.count(); ++i) {
        auto text = get_key_text(static_cast<Qt::Key>(m_key_sequence[i]));
        auto text_size = QSize(metrics.horizontalAdvance(text),
          metrics.height());
        draw_key(text, text_size, pos, painter);
        pos.setX(pos.x() + TEXT_PADDING() * 2 + text_size.width() +
          scale_width(4));
      }
    }
  } else {
    painter.setFont(m_font);
    painter.setPen(Qt::black);
    painter.drawText(scale_width(8), scale_height(16), tr("Enter Keys"));
  }
}

void KeySequenceInputField::commit_sequence(const QKeySequence& sequence) {
  m_key_sequence = sequence;
  m_entered_keys.clear();
  m_state = State::DEFAULT;
  update();
  Q_EMIT editingFinished();
}

void KeySequenceInputField::draw_key(const QString& text,
    const QSize& text_size, const QPoint& pos, QPainter& painter) const {
  auto path = QPainterPath();
  path.addRoundedRect(QRectF(pos.x(), pos.y() - scale_height(18) -
    scale_height(2) - 1, text_size.width() + TEXT_PADDING() * 2,
    scale_height(18)), scale_width(2), scale_height(2));
  painter.setPen({QColor("#E2C899"), static_cast<qreal>(scale_width(1))});
  painter.fillPath(path, QColor("#FFEDCD"));
  painter.drawPath(path);
  painter.setPen(Qt::black);
  painter.drawText(pos.x() + TEXT_PADDING(),
    pos.y() - (text_size.height() / 2), text);
}
