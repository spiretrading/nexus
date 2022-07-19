#include "Spire/Blotter/BlotterProfitAndLossView.hpp"
#include "Spire/Blotter/BlotterProfitAndLossModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/ToggleButton.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct CurrencyExpandButton : QWidget {
    std::shared_ptr<ValueModel<CurrencyId>> m_currency;
    std::shared_ptr<BooleanModel> m_is_expanded;

    CurrencyExpandButton(std::shared_ptr<ValueModel<CurrencyId>> currency,
        std::shared_ptr<BooleanModel> is_expanded)
        : m_currency(std::move(currency)),
          m_is_expanded(std::move(is_expanded)) {
      enclose(*this, *make_label(displayText(m_currency->get())));
    }
  };
}

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
  table_view_builder.set_view_builder([] (
      const std::shared_ptr<TableModel>& table, int row, int column) ->
        QWidget* {
    if(column == BlotterProfitAndLossModel::CURRENCY &&
        table->get<Security>(row, BlotterProfitAndLossModel::SECURITY) ==
          Security()) {
      auto index = std::make_shared<
        ListValueModel<BlotterProfitAndLossModel::CurrencyIndex>>(
          std::make_shared<ColumnViewListModel<
            BlotterProfitAndLossModel::CurrencyIndex>>(table, column), row);
      auto currency = make_field_value_model(
        index, &BlotterProfitAndLossModel::CurrencyIndex::m_index);
      auto is_expanded = make_field_value_model(
        index, &BlotterProfitAndLossModel::CurrencyIndex::m_is_expanded);
      return new CurrencyExpandButton(currency, is_expanded);
    }
    return TableView::default_view_builder(table, row, column);
  });
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
