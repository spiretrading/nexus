#include "Spire/Ui/RegionListItem.hpp"
#include <QHBoxLayout>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Button.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto ICON_SIZE() {
    static auto size = scale(10, 10);
    return size;
  }

  auto ICON_STYLE(StyleSheet style) {
    style.get(Any()).set(Fill(none));
    style.get(Hover()).set(Fill(none));
    return style;
  }

  auto NAME_LABEL_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor::fromRgb(0x8C, 0x8C, 0x8C))).
      set(PaddingTop(scale_height(4)));
    return style;
  }

  auto VALUE_LABEL_STYLE(StyleSheet style) {
    style.get(ReadOnly() && Disabled()).
      set(PaddingRight(scale_width(8)));
    return style;
  }
}

RegionListItem::RegionListItem(Nexus::Region region, QWidget* parent)
    : QWidget(parent),
      m_region(std::move(region)),
      m_type(Type::NONE) {
  auto value_label = make_value_label();
  value_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  set_style(*value_label, VALUE_LABEL_STYLE(get_style(*value_label)));
  auto name_label = make_label(QString::fromStdString(m_region.GetName()));
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  set_style(*name_label, NAME_LABEL_STYLE(get_style(*name_label)));
  auto type_icon = make_type_icon();
  auto value_container_layout = new QHBoxLayout();
  value_container_layout->setContentsMargins({});
  value_container_layout->setSpacing(0);
  value_container_layout->addWidget(value_label);
  if(type_icon) {
    type_icon->setFocusPolicy(Qt::NoFocus);
    set_style(*type_icon, ICON_STYLE(get_style(*type_icon)));
    value_container_layout->addWidget(type_icon);
  }
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addLayout(value_container_layout);
  layout->addWidget(name_label);
}

const Nexus::Region& RegionListItem::get_region() const {
  return m_region;
}

TextBox* RegionListItem::make_value_label() {
  if(!m_region.GetSecurities().empty()) {
    m_type = Type::SECURITY;
    return make_label(
      QString::fromStdString(m_region.GetSecurities().begin()->GetSymbol()));
  } else if(!m_region.GetMarkets().empty()) {
    m_type = Type::MARKET;
    return make_label(
      QString::fromStdString(m_region.GetMarkets().begin()->GetData()));
  } else if(!m_region.GetCountries().empty()) {
    m_type = Type::COUNTRY;
    return make_label(GetDefaultCountryDatabase().
      FromCode(*m_region.GetCountries().begin()).m_twoLetterCode.GetData());
  }
  m_type = Type::NONE;
  return make_label("");
}

Icon* RegionListItem::make_type_icon() {
  if(m_type == Type::SECURITY) {
    auto icon = new Icon(
      imageFromSvg(":/Icons/security-symbol.svg", ICON_SIZE()), this);
    icon->setFixedSize(ICON_SIZE());
    return icon;
  } else if(m_type == Type::MARKET) {
    auto icon = new Icon(
      imageFromSvg(":/Icons/market-symbol.svg", ICON_SIZE()), this);
    icon->setFixedSize(ICON_SIZE());
    return icon;
  } else if(m_type == Type::COUNTRY) {
    auto flag_icon = new Icon(QImage(QString(":/Icons/%1.png").
      arg(std::uint16_t(*m_region.GetCountries().begin()))));
    flag_icon->setFixedSize(scale(16, 10));
    return flag_icon;
  }
  return nullptr;
}
