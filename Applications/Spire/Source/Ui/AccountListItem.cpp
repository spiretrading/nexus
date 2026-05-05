#include "Spire/Ui/AccountListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto ICON_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(none));
    return style;
  }
}

AccountListItem::AccountListItem(Account account, QWidget* parent)
    : QWidget(parent),
      m_account(std::move(account)) {
  auto icon = new Icon(m_account.m_identicon);
  icon->setFixedSize(scale(8, 8));
  icon->setFocusPolicy(Qt::NoFocus);
  set_style(*icon, ICON_STYLE());
  auto icon_box = new Box(icon);
  icon_box->setFixedSize(scale(14, 14));
  update_style(*icon_box, [] (auto& style) {
    style.get(Any()).
      set(PaddingRight(scale_width(6))).
      set(vertical_padding(scale_height(3)));
  });
  auto id_label = make_label(m_account.m_id);
  id_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto name_label = make_label(m_account.m_name);
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*name_label, [] (auto& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingBottom(scale_height(2))).
      set(PaddingTop(scale_height(3)));
  });
  auto layout = make_grid_layout(this);
  layout->addWidget(icon_box, 0, 0);
  layout->addWidget(id_label, 0, 1);
  layout->addWidget(name_label, 1, 1);
}

const AccountListItem::Account& AccountListItem::get_account() const {
  return m_account;
}
