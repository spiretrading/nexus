#include "Spire/Ui/KeyTag.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DEFAULT_COLOR = QColor("#C5EBC4");
  const auto ESCAPE_BACKGROUND_COLOR = QColor("#C6E6FF");
  const auto MODIFIER_BACKGROUND_COLOR = QColor("#FFEDCD");

  auto TAG_STYLE(StyleSheet style) {
    style.get(Disabled()).
      set(border_radius(scale_width(3))).
      set(border_size(0)).
      set(PaddingLeft(scale_width(4))).
      set(PaddingRight(scale_width(4))).
      set(PaddingBottom(scale_height(2))).
      set(PaddingTop(scale_height(2))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(TextColor(QColor::fromRgb(0, 0, 0)));
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

  auto tag_background_color(Qt::Key key) {
    switch(key) {
      case Qt::Key_Alt:
      case Qt::Key_Control:
      case Qt::Key_Shift:
        return MODIFIER_BACKGROUND_COLOR;
      case Qt::Key_Escape:
        return ESCAPE_BACKGROUND_COLOR;
      default:
        return DEFAULT_COLOR;
    }
  }
}

KeyTag::KeyTag(QWidget* parent)
  : KeyTag(std::make_shared<LocalKeyModel>(Qt::Key_unknown), parent) {}

KeyTag::KeyTag(std::shared_ptr<KeyModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  setObjectName("key_tag");
  setStyleSheet("#key_tag { background-color: transparent; }");
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(this);
  m_text_box->setDisabled(true);
  set_style(*m_text_box, TAG_STYLE(get_style(*m_text_box)));
  layout->addWidget(m_text_box);
  m_current_connection = m_model->connect_current_signal([=] (auto key) {
    on_current_key(key);
  });
  on_current_key(m_model->get_current());
}

const std::shared_ptr<KeyModel>& KeyTag::get_model() const {
  return m_model;
}

QSize KeyTag::sizeHint() const {
  return m_text_box->sizeHint();
}

void KeyTag::on_current_key(Qt::Key key) {
  m_text_box->get_model()->set_current(get_key_text(key));
  auto style = get_style(*m_text_box);
  style.get(Disabled()).
    set(BackgroundColor(tag_background_color(key)));
  set_style(*m_text_box, style);
}
