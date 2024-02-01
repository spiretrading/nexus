#include "Spire/Ui/SaleConditionListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto NAME_LABEL_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingTop(scale_height(2)));
    return style;
  }
}

SaleConditionListItem::SaleConditionListItem(SaleConditionInfo condition_info,
    QWidget* parent)
    : QWidget(parent),
      m_condition_info(std::move(condition_info)) {
  auto code_label = make_label(
    QString::fromStdString(m_condition_info.m_condition.m_code));
  code_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto name_label = make_label(m_condition_info.m_name);
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*name_label, [] (auto& style) {
    style = NAME_LABEL_STYLE(style);
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(code_label);
  layout->addWidget(name_label);
}

const SaleConditionInfo& SaleConditionListItem::get_condition() const {
  return m_condition_info;
}
