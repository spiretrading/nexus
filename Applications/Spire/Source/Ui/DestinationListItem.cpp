#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
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

DestinationListItem::DestinationListItem(DestinationDatabase::Entry destination,
    QWidget* parent)
    : QWidget(parent),
      m_destination(std::move(destination)) {
  auto value_label = make_label(QString::fromStdString(m_destination.m_id));
  value_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto layout = make_vbox_layout(this);
  layout->addWidget(value_label);
  auto name_label = make_label(
    QString::fromStdString(m_destination.m_description));
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*name_label, [&] (auto& style) {
    style = NAME_LABEL_STYLE(style);
  });
  layout->addWidget(name_label);
}

const DestinationDatabase::Entry& DestinationListItem::get_destination() const {
  return m_destination;
}
