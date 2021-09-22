#include "Spire/Ui/KeyTag.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
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

KeyTag::KeyTag(std::shared_ptr<KeyModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_state(State::DEFAULT) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_label = make_label("");
  proxy_style(*this, *m_label);
  set_style(*m_label, TAG_STYLE(get_style(*m_label)));
  auto style = get_style(*this);
  style.get(Any()).
    set(BackgroundColor(DEFAULT_BACKGROUND_COLOR));
  style.get(ModifierKeyState()).
    set(BackgroundColor(MODIFIER_BACKGROUND_COLOR));
  style.get(EscapeKeyState()).
    set(BackgroundColor(ESCAPE_BACKGROUND_COLOR));
  set_style(*this, std::move(style));
  layout->addWidget(m_label);
  m_current_connection = m_model->connect_current_signal([=] (auto key) {
    on_current_key(key);
  });
  on_current_key(m_model->get_current());
}

const std::shared_ptr<KeyModel>& KeyTag::get_model() const {
  return m_model;
}

void KeyTag::on_current_key(Qt::Key key) {
  m_label->get_model()->set_current(get_key_text(key));
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
