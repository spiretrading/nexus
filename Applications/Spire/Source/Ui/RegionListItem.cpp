#include "Spire/Ui/RegionListItem.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto ICON_SIZE() {
    static auto size = scale(10, 10);
    return size;
  }

  auto FLAG_SIZE() {
    static auto size = scale(16, 10);
    return size;
  }

  auto ICON_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(none));
    return style;
  }

  auto NAME_LABEL_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor::fromRgb(0x80, 0x80, 0x80))).
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

RegionListItem::RegionListItem(Region region, QWidget* parent)
    : QWidget(parent),
      m_region(std::move(region)),
      m_type(get_type()) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto value_container_layout = new QHBoxLayout();
  value_container_layout->setContentsMargins({});
  value_container_layout->setSpacing(0);
  auto value_label = make_value_label();
  value_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  set_style(*value_label, VALUE_LABEL_STYLE(get_style(*value_label)));
  value_container_layout->addWidget(value_label);
  auto type_icon_layout = new QVBoxLayout();
  type_icon_layout->setContentsMargins({});
  type_icon_layout->setSpacing(0);
  type_icon_layout->addStretch();
  if(auto type_icon = make_type_icon()) {
    type_icon->setFocusPolicy(Qt::NoFocus);
    set_style(*type_icon, ICON_STYLE());
    type_icon_layout->addWidget(type_icon);
  }
  type_icon_layout->addStretch();
  value_container_layout->addLayout(type_icon_layout);
  layout->addLayout(value_container_layout);
  auto name_label = make_label(QString::fromStdString(m_region.GetName()));
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  set_style(*name_label, NAME_LABEL_STYLE(get_style(*name_label)));
  layout->addWidget(name_label);
}

const Region& RegionListItem::get_region() const {
  return m_region;
}

RegionListItem::Type RegionListItem::get_type() const {
  if(!m_region.GetSecurities().empty()) {
    return Type::SECURITY;
  } else if(!m_region.GetMarkets().empty()) {
    return Type::MARKET;
  } else if(!m_region.GetCountries().empty()) {
    return Type::COUNTRY;
  }
  return Type::NONE;
}

TextBox* RegionListItem::make_value_label() const {
  if(m_type == Type::SECURITY) {
    return make_label(
      QString::fromStdString(ToString(*m_region.GetSecurities().begin())));
  } else if(m_type == Type::MARKET) {
    return make_label(
      QString::fromStdString(m_region.GetMarkets().begin()->GetData()));
  } else if(m_type == Type::COUNTRY) {
    return make_label(GetDefaultCountryDatabase().
      FromCode(*m_region.GetCountries().begin()).m_threeLetterCode.GetData());
  }
  return make_label("");
}

Icon* RegionListItem::make_type_icon() const {
  if(m_type == Type::SECURITY) {
    auto icon = new Icon(
      imageFromSvg(":/Icons/security-symbol.svg", ICON_SIZE()));
    icon->setFixedSize(ICON_SIZE());
    return icon;
  } else if(m_type == Type::MARKET) {
    auto icon = new Icon(
      imageFromSvg(":/Icons/market-symbol.svg", ICON_SIZE()));
    icon->setFixedSize(ICON_SIZE());
    return icon;
  } else if(m_type == Type::COUNTRY) {
    auto country_code = QString(GetDefaultCountryDatabase().
      FromCode(*m_region.GetCountries().begin()).
      m_threeLetterCode.GetData()).toLower();
    auto flag_icon = new Icon(imageFromSvg(QString(":/Icons/flag_icons/%1.svg").
      arg(country_code), FLAG_SIZE()));
    flag_icon->setFixedSize(FLAG_SIZE());
    return flag_icon;
  }
  return nullptr;
}
