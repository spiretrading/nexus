#include "Spire/Blotter/BlotterProfitAndLossView.hpp"
#include "Spire/Blotter/BlotterProfitAndLossModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterProfitAndLossView::BlotterProfitAndLossView(
    std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
    QWidget* parent)
    : QWidget(parent),
      m_profit_and_loss(std::move(profit_and_loss)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder = TableViewBuilder(m_profit_and_loss);
  table_view_builder.add_header_item(tr("Currency"), tr("Curr"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Profit/Loss"), tr("P/L"));
  table_view_builder.add_header_item(tr("Fees"));
  table_view_builder.add_header_item(tr("Volume"), tr("Vol"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  enclose(*this, *scroll_box);
}

const std::shared_ptr<BlotterProfitAndLossModel>&
    BlotterProfitAndLossView::get_profit_and_loss() const {
  return m_profit_and_loss;
}
