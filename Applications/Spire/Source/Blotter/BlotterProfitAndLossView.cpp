#include "Spire/Blotter/BlotterProfitAndLossView.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_table(std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss) {
    return std::make_shared<ArrayTableModel>();
  }
}

BlotterProfitAndLossView::BlotterProfitAndLossView(
    std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
    QWidget* parent)
    : QWidget(parent),
      m_profit_and_loss(std::move(profit_and_loss)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder = TableViewBuilder(to_table(m_profit_and_loss));
  table_view_builder.add_header_item(tr("Currency"), tr("Curr"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Profit/Loss"), tr("P/L"));
  table_view_builder.add_header_item(tr("Fees"));
  table_view_builder.add_header_item(tr("Volume"), tr("Vol"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table);
}

const std::shared_ptr<BlotterProfitAndLossModel>&
    BlotterProfitAndLossView::get_profit_and_loss() const {
  return m_profit_and_loss;
}
