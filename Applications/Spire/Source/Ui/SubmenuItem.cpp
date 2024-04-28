#include "Spire/Ui/SubmenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0x333333)));
    style.get(Any() > is_a<TextBox>()).
      set(PaddingRight(scale_width(14)));
    style.get(Disabled() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0xC8C8C8)));
    style.get(Disabled() > is_a<TextBox>()).
      set(Fill(QColor(0xC8C8C8)));
    return style;
  }
}

SubmenuItem::SubmenuItem(QString label, ContextMenu& menu, QWidget* parent)
    : QWidget(parent),
      m_menu(&menu) {
  auto label_box = make_label(label);
  label_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto layout = make_hbox_layout(this);
  layout->addWidget(label_box);
  auto icon = new Icon(imageFromSvg(":/Icons/menu-arrow.svg", scale(4, 6)));
  icon->setFixedSize(scale(4, 6));
  layout->addWidget(icon, 0, Qt::AlignVCenter);
  set_style(*this, DEFAULT_STYLE());
}

ContextMenu& SubmenuItem::get_context_menu() {
  return *m_menu;
}
