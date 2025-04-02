#include "Spire/Ui/DeletableListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

DeletableListItem::DeletableListItem(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body) {
  auto delete_button =
    make_icon_button(imageFromSvg(":/Icons/delete2.svg", scale(26, 24)));
  delete_button->setFocusPolicy(Qt::NoFocus);
  delete_button->setFixedSize(scale(26, 24));
  update_style(*delete_button, [] (auto& style) {
    style.get(Any()).set(Visibility(Visibility::NONE));
    style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0xB71C1C)));
    style.get(Press() > is_a<Icon>()).set(Fill(QColor(0xE01616)));
  });
  auto content = new Box(m_body);
  content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*content, [] (auto& style) {
    style.get(Any()).
      set(vertical_padding(scale_height(5))).
      set(horizontal_padding(scale_width(8)));
  });
  auto layout = make_hbox_layout(this);
  layout->setSpacing(scale_width(4));
  layout->addWidget(content, 0, Qt::AlignVCenter);
  layout->addWidget(delete_button, 0, Qt::AlignVCenter);
  update_style(*this, [] (auto& style) {
    style.get(Hover() > is_a<Button>()).set(Visibility::VISIBLE);
    style.get(Hover() > (is_a<Button>() && (Hover() || Press())) > Body()).
      set(BackgroundColor(QColor(0xDFDFEB)));
  });
  delete_button->connect_click_signal(m_delete_signal);
}

QWidget& DeletableListItem::get_body() {
  return *m_body;
}

connection DeletableListItem::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}
