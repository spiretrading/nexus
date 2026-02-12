#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

FilterPanel::FilterPanel(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body) {
  auto box_body = new QWidget();
  box_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto layout = make_hbox_layout(box_body);
  layout->addWidget(m_body, 1);
  layout->addStretch();
  layout->addSpacing(scale_width(18));
  auto reset_button = make_icon_button(
    image_from_svg(":/Icons/reset.svg", scale(26, 26)));
  reset_button->setFixedSize(scale(26, 26));
  reset_button->connect_click_signal([=] { m_reset_signal(); });
  layout->addWidget(reset_button);
  auto box = new Box(box_body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, [&] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
}

const QWidget& FilterPanel::get_body() const {
  return *m_body;
}

QWidget& FilterPanel::get_body() {
  return *m_body;
}

connection FilterPanel::connect_reset_signal(
    const ResetSignal::slot_type& slot) const {
  return m_reset_signal.connect(slot);
}
