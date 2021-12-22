#include "Spire/Ui/TableHeaderCell.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

TableHeaderCell::TableHeaderCell(
    std::shared_ptr<CompositeValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto name_label = make_label(m_model->get(&Model::m_name));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(name_label);
  auto style = StyleSheet();
  style.get(Any() > is_a<TextBox>()).set(TextColor(QColor(0x808080)));
  style.get(Hover() > is_a<TextBox>()).set(TextColor(QColor(0x4B23A0)));
  set_style(*this, std::move(style));
}

const std::shared_ptr<CompositeValueModel<TableHeaderCell::Model>>&
    TableHeaderCell::get_model() const {
  return m_model;
}

connection TableHeaderCell::connect_hide_signal(
    const HideSignal::slot_type& slot) const {
  return m_hide_signal.connect(slot);
}

connection TableHeaderCell::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}
