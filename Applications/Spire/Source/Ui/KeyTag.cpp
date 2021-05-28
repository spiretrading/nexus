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

  auto TAG_STYLE() {
    auto style = StyleSheet();

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

  auto set_tag_style(TextBox* text_box, Qt::Key key) {
    text_box->get_model()->set_current(get_key_text(key));
    auto style = get_style(*text_box);
    style.get(Any()).
      set(BackgroundColor(tag_background_color(key))).
      set(border_radius(scale_width(3))).
      set(PaddingLeft(scale_width(4))).
      set(PaddingRight(scale_width(4))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(TextColor(QColor::fromRgb(0, 0, 0)));
    set_style(*text_box, style);
  }
}

KeyTag::KeyTag(QWidget* parent)
  : KeyTag(std::make_shared<LocalKeyModel>(Qt::Key_unknown), parent) {}

KeyTag::KeyTag(std::shared_ptr<KeyModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(this);
  set_style(*m_text_box, TAG_STYLE());
  m_text_box->set_read_only(true);
  m_text_box->setDisabled(true);
  layout->addWidget(m_text_box);
  m_current_connection = m_model->connect_current_signal([=] (auto key) {
    set_tag_style(m_text_box, key);
    m_text_box->adjustSize();
    adjustSize();
  });
  set_tag_style(m_text_box, m_model->get_current());
}

const std::shared_ptr<KeyModel>& KeyTag::get_model() const {
  return m_model;
}
