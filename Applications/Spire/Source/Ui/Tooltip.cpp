#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::black))).
      set(border_size(0));
    auto font = QFont("Roboto");
    font.setWeight(60);
    font.setPixelSize(scale_width(10));
    style.get(Any() > is_a<TextBox>()).
      set(Font(font)).
      set(TextColor(QColor(0xFFFFFF))).
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
  m_label->get_current()->set(text);
  m_tooltip->adjustSize();
  m_tooltip->setDisabled(text.isEmpty());
  if(!m_tooltip->isEnabled()) {
    m_tooltip->hide();
  }
}

void Spire::add_tooltip(QString text, QWidget& parent) {
  if(!text.isEmpty()) {
    new Tooltip(std::move(text), &parent);
  }
}
