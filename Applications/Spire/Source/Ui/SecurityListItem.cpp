#include "Spire/Ui/SecurityListItem.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto FLAG_SIZE() {
    static auto size = scale(16, 10);
    return size;
  }

  auto FLAG_ICON_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(none));
    return style;
  }

  auto NAME_LABEL_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingBottom(0)).
      set(PaddingTop(scale_height(4)));
    return style;
  }

  auto VALUE_LABEL_STYLE(StyleSheet style) {
    style.get(ReadOnly() && Disabled()).
      set(PaddingRight(scale_width(8))).
      set(vertical_padding(0));
    return style;
  }
}

SecurityListItem::SecurityListItem(SecurityInfo security_info, QWidget* parent)
    : QWidget(parent),
      m_security_info(std::move(security_info)) {
  auto value_label = make_label(
    QString::fromStdString(ToString(m_security_info.m_security)));
  value_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*value_label, [&] (auto& style) {
    style = VALUE_LABEL_STYLE(style);
  });
  auto value_container_layout = make_hbox_layout();
  value_container_layout->addWidget(value_label);
  auto country_code = QString(GetDefaultCountryDatabase().
    FromCode(m_security_info.m_security.GetCountry()).
    m_threeLetterCode.GetData()).toLower();
  auto flag_icon = new Icon(imageFromSvg(QString(":/Icons/flag_icons/%1.svg").
    arg(country_code), FLAG_SIZE()), this);
  flag_icon->setFixedSize(FLAG_SIZE());
  set_style(*flag_icon, FLAG_ICON_STYLE());
  flag_icon->setFocusPolicy(Qt::NoFocus);
  auto flag_icon_layout = make_vbox_layout();
  flag_icon_layout->addStretch();
  flag_icon_layout->addWidget(flag_icon);
  flag_icon_layout->addStretch();
  value_container_layout->addLayout(flag_icon_layout);
  auto layout = make_vbox_layout(this);
  layout->addLayout(value_container_layout);
  auto name_label = make_label(QString::fromStdString(m_security_info.m_name));
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*name_label, [&] (auto& style) {
    style = NAME_LABEL_STYLE(style);
  });
  layout->addWidget(name_label);
}

const SecurityInfo& SecurityListItem::get_security() const {
  return m_security_info;
}
