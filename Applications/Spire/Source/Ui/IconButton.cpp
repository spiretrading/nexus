#include "Spire/Ui/IconButton.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

//IconButton::Style::Style()
//  : m_blur_color("#7F5EEC"),
//    m_checked_blur_color("#1FD37A"),
//    m_checked_color(m_checked_blur_color),
//    m_checked_hovered_color("#2CAC79"),
//    m_default_color(m_blur_color),
//    m_disabled_color("#D0D0D0"),
//    m_disabled_background_color("#F5F5F5"),
//    m_hover_color("#4B23A0"),
//    m_default_background_color(m_disabled_background_color),
//    m_hover_background_color("#E3E3E3") {}

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();

    return style;
  }
}

IconButton::IconButton(Icon* icon, QWidget* parent)
    : QWidget(parent),
      m_tooltip(nullptr) {
  m_button = new Button(icon, this);
  auto box = new Box(m_button, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(box);
  set_style(*this, DEFAULT_STYLE);
}

bool IconButton::event(QEvent* event) {
  if(event->type() == QEvent::ToolTipChange) {
    delete_later(m_tooltip);
    if(!toolTip().isEmpty()) {
      m_tooltip = make_text_tooltip(toolTip(), this);
    }
  }
  return QWidget::event(event);
}

connection IconButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_button->connect_clicked_signal(slot);
}
