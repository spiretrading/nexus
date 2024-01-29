#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DEFAULT_BACKGROUND_COLOR = QColor(0xC5EBC4);
  const auto ESCAPE_BACKGROUND_COLOR = QColor(0xC6E6FF);
  const auto MODIFIER_BACKGROUND_COLOR = QColor(0xFFEDCD);

  auto TAG_STYLE(StyleSheet style) {
    style.get(ReadOnly() && Disabled()).
      set(border_radius(scale_width(3))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(4))).
      set(vertical_padding(scale_height(2)));
    return style;
  }

  auto get_key_text(Qt::Key key) {
    switch(key) {
      case Qt::Key_Shift:
        return QObject::tr("SHIFT");
      case Qt::Key_Alt:
        return QObject::tr("ALT");
      case Qt::Key_Control:
        return QObject::tr("CTRL");
      case Qt::Key_PageDown:
        return QObject::tr("PGDN");
      case Qt::Key_Up:
        return QObject::tr("ARROWUP");
      case Qt::Key_Down:
        return QObject::tr("ARROWDN");
      case Qt::Key_Left:
        return QObject::tr("ARROWLFT");
      case Qt::Key_Right:
        return QObject::tr("ARROWRHT");
      case Qt::Key_unknown:
        return QObject::tr("NaK");
      default:
        return QKeySequence(key).toString().toUpper();
    }
  }
}

KeyTag::KeyTag(QWidget* parent)
  : KeyTag(std::make_shared<LocalKeyModel>(Qt::Key_unknown), parent) {}

KeyTag::KeyTag(std::shared_ptr<KeyModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_state(State::DEFAULT) {
  setFocusPolicy(Qt::NoFocus);
  m_label = make_label("");
  enclose(*this, *m_label);
  proxy_style(*this, *m_label);
  update_style(*m_label, [&] (auto& style) {
    style = TAG_STYLE(style);
  });
  update_style(*this, [&] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(DEFAULT_BACKGROUND_COLOR));
    style.get(ModifierKeyState()).
      set(BackgroundColor(MODIFIER_BACKGROUND_COLOR));
    style.get(EscapeKeyState()).
      set(BackgroundColor(ESCAPE_BACKGROUND_COLOR));
  });
  m_current_connection = m_current->connect_update_signal([=] (auto key) {
    on_current(key);
  });
  on_current(m_current->get());
}

const std::shared_ptr<KeyModel>& KeyTag::get_current() const {
  return m_current;
}

void KeyTag::on_current(Qt::Key key) {
  m_label->get_current()->set(get_key_text(key));
  switch(key) {
    case Qt::Key_Alt:
    case Qt::Key_Control:
    case Qt::Key_Shift:
      if(m_state == State::ESCAPE) {
        unmatch(*this, EscapeKeyState());
      }
      match(*this, ModifierKeyState());
      m_state = State::MODIFIER;
      break;
    case Qt::Key_Escape:
      if(m_state == State::MODIFIER) {
        unmatch(*this, ModifierKeyState());
      }
      match(*this, EscapeKeyState());
      m_state = State::ESCAPE;
      break;
    default:
      if(m_state == State::MODIFIER) {
        unmatch(*this, ModifierKeyState());
      } else if(m_state == State::ESCAPE) {
        unmatch(*this, EscapeKeyState());
      }
      m_state = State::DEFAULT;
  }
}
