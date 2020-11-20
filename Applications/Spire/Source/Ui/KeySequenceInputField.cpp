#include "Spire/Ui/KeySequenceInputField.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/KeySequenceValidationModel.hpp"

using namespace Spire;

namespace {
  const auto& FONT() {
    static auto font = [&] {
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      font.setStyle(QFont::StyleItalic);
      return font;
    }();
    return font;
  }

  const auto& TEXT_PADDING() {
    static auto padding = scale_width(4);
    return padding;
  }

  auto get_key_color(Qt::Key key, bool is_enabled) {
    switch(key) {
      case Qt::Key_Alt:
        if(is_enabled) {
          return QColor("#C6A600");
        }
        return QColor("#E3D68F");
      case Qt::Key_Control:
        if(is_enabled) {
          return QColor("#00B395");
        }
        return QColor("#A6E4DA");
      case Qt::Key_Shift:
        if(is_enabled) {
          return QColor("#4495FF");
        }
        return QColor("#B0D2FF");
    }
    if(is_enabled) {
      return QColor("#684BC7");
    }
    return QColor("#C0B3E9");
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


  QKeySequence sort_key_sequence(const QKeySequence& sequence) {
    auto modifier_list = std::vector<Qt::Key>();
    auto function_list = std::vector<Qt::Key>();
    for(auto i = 0; i < sequence.count(); ++i) {
      auto key = Qt::Key(sequence[i]);
      if(key == Qt::Key_Control || key == Qt::Key_Alt ||
          key == Qt::Key_Shift) {
        modifier_list.push_back(key);
      } else {
        function_list.push_back(key);
      }
    }
    if(modifier_list.empty()) {
      return make_key_sequence(function_list);
    } else if(modifier_list.size() > 1) {
      std::sort(modifier_list.begin(), modifier_list.end(),
        [] (auto key1, auto key2) {
          if(key1 == Qt::Key_Control) {
            return true;
          } else if(key2 == Qt::Key_Control) {
            return false;
          }
          return key1 == Qt::Key_Alt;
        });
    }
    modifier_list.insert(modifier_list.end(), function_list.begin(),
      function_list.end());
    return make_key_sequence(modifier_list);
  }
}

KeySequenceInputField::KeySequenceInputField(
    std::shared_ptr<KeySequenceValidationModel> model, QWidget* parent)
    : QLineEdit(parent),
      m_model(std::move(model)),
      m_state(State::DEFAULT),
      m_last_pressed_key(Qt::Key_unknown),
      m_is_last_key_event_release(false) {
  setReadOnly(true);
  setAttribute(Qt::WA_Hover);
  installEventFilter(this);
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

bool KeySequenceInputField::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) &&
        !e->isAutoRepeat()) {
      if(m_state == State::EDIT) {
        m_entered_keys.push_back(Qt::Key(e->key()));
        return true;
      }
    }
  } else if(event->type() == QEvent::KeyRelease) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
      if(m_state == State::EDIT && !e->isAutoRepeat()) {
        commit_sequence(make_key_sequence(m_entered_keys));
        m_is_last_key_event_release = true;
        return true;
      }
    }
  }
  return QLineEdit::eventFilter(watched, event);
}

void KeySequenceInputField::focusOutEvent(QFocusEvent* event) {
  m_is_last_key_event_release = false;
  m_last_pressed_key = Qt::Key_unknown;
  m_state = State::DEFAULT;
  m_entered_keys.clear();
  QLineEdit::focusOutEvent(event);
}

void KeySequenceInputField::keyPressEvent(QKeyEvent* event) {
  if(event->isAutoRepeat()) {
    return;
  }
  m_is_last_key_event_release = false;
  m_last_pressed_key = Qt::Key(event->key());
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(m_state == State::DEFAULT) {
        m_state = State::EDIT;
      } else {
        m_state = State::DEFAULT;
        m_entered_keys.clear();
      }
      update();
      return;
    case Qt::Key_Delete:
      m_state = State::DEFAULT;
      commit_sequence({});
      return;
  }
  m_entered_keys.push_back(static_cast<Qt::Key>(event->key()));
}

void KeySequenceInputField::keyReleaseEvent(QKeyEvent* event) {
  if(event->isAutoRepeat()) {
    return;
  }
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      return;
    case Qt::Key_Delete:
      m_state = State::DEFAULT;
      commit_sequence({});
  }
  if(!m_is_last_key_event_release && (m_last_pressed_key != Qt::Key_unknown)) {
    auto key_sequence = make_key_sequence(m_entered_keys);
    if(m_model->is_valid(key_sequence)) {
      commit_sequence(key_sequence);
    }
  }
  m_is_last_key_event_release = true;
}

void KeySequenceInputField::mousePressEvent(QMouseEvent* event) {
  m_last_pressed_key = Qt::Key_unknown;
  if(event->button() == Qt::LeftButton) {
    m_entered_keys.clear();
    m_state = State::EDIT;
    update();
  }
}

void KeySequenceInputField::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, scale_height(1), width(), height(), Qt::white);
  if(hasFocus() || (underMouse() && isEnabled())) {
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
        auto text_size = QSize(metrics.horizontalAdvance(
          get_key_text(Qt::Key(m_key_sequence[i]))), metrics.height());
        draw_key(Qt::Key(m_key_sequence[i]), text_size, pos, painter);
        pos.setX(pos.x() + TEXT_PADDING() * 2 + text_size.width() +
          scale_width(4));
      }
    }
  } else {
    painter.setFont(FONT());
    painter.setPen(Qt::black);
    painter.drawText(scale_width(8), scale_height(16), tr("Enter Keys"));
  }
}

void KeySequenceInputField::commit_sequence(const QKeySequence& sequence) {
  m_key_sequence = sort_key_sequence(sequence);
  m_entered_keys.clear();
  m_state = State::DEFAULT;
  update();
  Q_EMIT editingFinished();
}

void KeySequenceInputField::draw_key(Qt::Key key, const QSize& text_size,
    const QPoint& pos, QPainter& painter) const {
  auto path = QPainterPath();
  path.addRoundedRect(QRectF(pos.x(), pos.y() - scale_height(15) -
    scale_height(4) - 1, text_size.width() + TEXT_PADDING() * 2,
    scale_height(15)), scale_width(1), scale_height(1));
  auto key_color = get_key_color(key, isEnabled());
  painter.setPen({key_color, static_cast<qreal>(scale_width(1))});
  painter.fillPath(path, key_color);
  painter.drawPath(path);
  painter.setPen(Qt::white);
  painter.drawText(pos.x() + TEXT_PADDING(),
    pos.y() - (text_size.height() / 2), get_key_text(key));
}
