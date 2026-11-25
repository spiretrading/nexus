#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <QFileDialog>
#include <QStandardPaths>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class ProfitLossColumn {
    CURRENCY,
    TICKER,
    PROFIT_LOSS,
    FEES,
    VOLUME
  };

  Qt::Alignment get_alignment(ProfitLossColumn column) {
    if(column == ProfitLossColumn::CURRENCY ||
        column == ProfitLossColumn::TICKER) {
      return Qt::AlignCenter;
    }
    return Qt::AlignRight | Qt::AlignVCenter;
  }

  AnyRef extract(const PortfolioUpdateEntry& entry, int index) {
    auto column = static_cast<ProfitLossColumn>(index);
    auto& inventory = entry.m_inventory;
    if(column == ProfitLossColumn::CURRENCY) {
      return inventory.m_position.m_currency;
    } else if(column == ProfitLossColumn::TICKER) {
      return inventory.m_position.m_ticker;
    } else if(column == ProfitLossColumn::PROFIT_LOSS) {
      return get_realized_profit_and_loss(inventory) + entry.m_unrealized;
    } else if(column == ProfitLossColumn::FEES) {
      return inventory.m_fees;
    }
    return inventory.m_volume;
  }

  QWidget* make_item(
      const std::shared_ptr<TableModel>& table, int row, int index) {
    auto column = static_cast<ProfitLossColumn>(index);
    auto item = [&] () -> QWidget* {
      if(column == ProfitLossColumn::PROFIT_LOSS) {
        auto label = make_money_label(make_scalar_value_model_decorator(
          make_table_value_model<Money>(table, row, index)));
        update_style(*label, [] (auto& style) {
          style.get(IsPositive()).set(TextColor(QColor(0x1B9449)));
          style.get(IsNegative()).set(TextColor(QColor(0xE01616)));
        });
        return label;
      }
      return TableView::default_item_builder(table, row, index);
    }();
    update_style(*item, [=] (auto& style) {
      auto alignment = get_alignment(column);
      style.get(Any()).set(TextAlign(alignment));
      if(column == ProfitLossColumn::CURRENCY) {
        style.get(Any()).set(PaddingLeft(scale_width(10)));
      } else if(alignment.testFlag(Qt::AlignRight)) {
        style.get(Any()).set(PaddingRight(scale_width(4)));
      }
    });
    return item;
  }

  void export_profit_loss(TableView& view) {
    auto file_name = QFileDialog::getSaveFileName(&view, QObject::tr("Save As"),
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      "/profit_loss.csv", QObject::tr("CSV (*.csv)"));
    if(file_name.isEmpty()) {
      return;
    }
    auto& header = view.get_header();
    auto headers = std::vector<QString>(header.get_items()->get_size());
    for(auto i = 0; i != header.get_items()->get_size(); ++i) {
      headers[header.get_column_order()->get(i)] =
        header.get_items()->get(i).m_name;
    }
    auto out = std::ofstream(std::filesystem::path(file_name.toStdWString()));
    export_as_csv(*view.get_table(), headers, out);
  }
}

BlotterProfitLossView::BlotterProfitLossView(
    std::shared_ptr<PortfolioModel> portfolio, QWidget* parent)
    : QWidget(parent),
      m_portfolio(std::move(portfolio)),
      m_table(nullptr) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table = std::make_shared<ListToTableModel<PortfolioUpdateEntry>>(
    m_portfolio, static_cast<int>(ProfitLossColumn::VOLUME) + 1, &extract);
  m_table = TableViewBuilder(table).
    add_header_item(tr("Currency"), tr("Curr")).
    add_header_item(tr("Security"), TableHeaderItem::Order::ASCENDING).
    add_header_item(tr("Profit/Loss"), tr("P/L")).
    add_header_item(tr("Fees")).
    add_header_item(tr("Volume"), tr("Vol")).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListMultiSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())).
    set_item_builder(&make_item).
    make();
  const auto DEFAULT_WIDTHS = std::array{70, 90, 90, 70, 60};
  for(auto i = 0; i != table->get_column_size(); ++i) {
    m_table->get_header().get_widths()->set(i, scale_width(DEFAULT_WIDTHS[i]));
    update_style(*m_table->get_header().get_item(i), [=] (auto& style) {
      style.get(Any() > TableHeaderItem::Label()).
        set(TextAlign(get_alignment(static_cast<ProfitLossColumn>(i))));
      if(i == static_cast<int>(ProfitLossColumn::CURRENCY)) {
        style.get(Any()).set(PaddingLeft(scale_width(24)));
      }
    });
  }
  auto& body = m_table->get_body();
  body.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&body, &QWidget::customContextMenuRequested, this,
    &BlotterProfitLossView::on_context_menu);
  auto& header = m_table->get_header();
  header.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&header, &QWidget::customContextMenuRequested, this,
    &BlotterProfitLossView::on_header_menu);
  m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *m_table);
  setFocusProxy(m_table);
}

const std::shared_ptr<PortfolioModel>&
    BlotterProfitLossView::get_portfolio() const {
  return m_portfolio;
}

void BlotterProfitLossView::on_context_menu(const QPoint& position) {
  auto& body = m_table->get_body();
  auto menu = new ContextMenu(body);
  menu->add_action(tr("Export"), [=, this] {
    export_profit_loss(*m_table);
  });
  menu->window()->setAttribute(Qt::WA_DeleteOnClose);
  menu->window()->move(body.mapToGlobal(position));
  menu->window()->show();
}

void BlotterProfitLossView::on_header_menu(const QPoint& position) {
  auto& header = m_table->get_header();
  auto menu = new ContextMenu(header);
  auto columns = new ContextMenu(static_cast<QWidget&>(*menu));
  for(auto i = 0; i != header.get_items()->get_size(); ++i) {
    if(header.get_column_order()->get(i) ==
        static_cast<int>(ProfitLossColumn::CURRENCY)) {
      continue;
    }
    auto checked = columns->add_check_box(header.get_items()->get(i).m_name);
    checked->set(!header.get_item(i)->isHidden());
    checked->connect_update_signal([=, this] (auto is_checked) {
      if(is_checked) {
        m_table->show_column(i);
      } else {
        m_table->hide_column(i);
      }
    });
  }
  menu->add_menu(tr("Table Columns"), *columns);
  menu->window()->setAttribute(Qt::WA_DeleteOnClose);
  menu->window()->move(header.mapToGlobal(position));
  menu->window()->show();
}
