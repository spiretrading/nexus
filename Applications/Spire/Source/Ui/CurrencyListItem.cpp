#include "Spire/Ui/CurrencyListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

CurrencyListItem::CurrencyListItem(
    CurrencyDatabase::Entry entry, QWidget* parent)
    : QWidget(parent),
      m_entry(std::move(entry)) {
  auto code_label = make_label(
    QString::fromStdString(m_entry.m_code.get_data()));
  code_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto name_label = make_label(QString::fromStdString(m_entry.m_name));
  name_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*name_label, [] (auto& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(Any()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingTop(scale_height(2)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(code_label);
  layout->addWidget(name_label);
}

const CurrencyDatabase::Entry& CurrencyListItem::get_entry() const {
  return m_entry;
}
