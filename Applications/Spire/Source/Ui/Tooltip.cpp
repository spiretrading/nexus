#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0))).
      set(border_size(0));
    auto font = QFont("Roboto");
    font.setWeight(60);
    font.setPixelSize(scale_width(10));
    style.get(Any() >> is_a<TextBox>()).
      set(Font(font)).
      set(TextColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(horizontal_padding(scale_width(8)));
    return style;
  }
}

Tooltip::Tooltip(QString text, QWidget* parent)
    : QObject(parent) {
  m_label = make_label("", parent);
  m_tooltip = new InfoTip(m_label, parent);
  set_label(std::move(text));
  set_style(*m_tooltip, DEFAULT_STYLE());
}

void Tooltip::set_label(const QString& text) {
  m_label->get_model()->set_current(text);
  m_tooltip->adjustSize();
  m_tooltip->setDisabled(text.isEmpty());
  if(!m_tooltip->isEnabled()) {
    m_tooltip->hide();
  }
}
