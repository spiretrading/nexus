#include "Spire/Blotter/BlotterPositionsView.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <QFileDialog>
#include <QKeyEvent>
#include <QStandardPaths>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
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

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using FlattenState = BlotterPositionsCommandBar::FlattenState;

  enum class PositionColumn {
    TICKER,
    QUANTITY,
    SIDE,
    AVERAGE_PRICE,
    PROFIT_LOSS,
    COST_BASIS,
    CURRENCY
  };

  Qt::Alignment get_alignment(PositionColumn column) {
    if(column == PositionColumn::QUANTITY ||
        column == PositionColumn::AVERAGE_PRICE ||
        column == PositionColumn::PROFIT_LOSS ||
        column == PositionColumn::COST_BASIS) {
      return Qt::AlignRight | Qt::AlignVCenter;
    }
    return Qt::AlignCenter;
  }

  AnyRef extract(const PortfolioUpdateEntry& entry, int index) {
    auto column = static_cast<PositionColumn>(index);
    auto& position = entry.m_inventory.m_position;
    if(column == PositionColumn::TICKER) {
      return position.m_ticker;
    } else if(column == PositionColumn::QUANTITY) {
      return abs(position.m_quantity);
    } else if(column == PositionColumn::SIDE) {
      return PositionSide(get_side(position));
    } else if(column == PositionColumn::AVERAGE_PRICE) {
      return get_average_price(position);
    } else if(column == PositionColumn::PROFIT_LOSS) {
      return entry.m_unrealized;
    } else if(column == PositionColumn::COST_BASIS) {
      return position.m_cost_basis;
    }
    return position.m_currency;
  }

  QWidget* make_item(
      const std::shared_ptr<TableModel>& table, int row, int index) {
    auto column = static_cast<PositionColumn>(index);
    auto item = [&] () -> QWidget* {
      if(column == PositionColumn::PROFIT_LOSS) {
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
      if(alignment.testFlag(Qt::AlignRight)) {
        style.get(Any()).set(PaddingRight(scale_width(4)));
      }
    });
    return item;
  }

  void export_positions(TableView& view) {
    auto file_name = QFileDialog::getSaveFileName(&view, QObject::tr("Save As"),
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      "/positions.csv", QObject::tr("CSV (*.csv)"));
    if(file_name.isEmpty()) {
      return;
    }
    auto headers = std::vector<QString>();
    for(auto& item : std::as_const(*view.get_header().get_items())) {
      headers.push_back(item.m_name);
    }
    auto out = std::ofstream(std::filesystem::path(file_name.toStdWString()));
    export_as_csv(*view.get_table(), headers, out);
  }
}

BlotterPositionsView::BlotterPositionsView(
    std::shared_ptr<PortfolioModel> portfolio,
    std::shared_ptr<PositionKeyBindingsModel> bindings, QWidget* parent)
    : QWidget(parent),
      m_portfolio(std::move(portfolio)),
      m_positions(std::make_shared<FilteredListModel<PortfolioUpdateEntry>>(
        m_portfolio, [] (const auto& portfolio, auto index) {
          return portfolio.get(index).m_inventory.m_position.m_quantity == 0;
        })),
      m_bindings(std::move(bindings)),
      m_can_flatten(std::make_shared<LocalValueModel<FlattenState>>(
        FlattenState::NONE)),
      m_table(nullptr) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto commands = new BlotterPositionsCommandBar(m_can_flatten, m_bindings);
  commands->connect_flatten_signal(
    std::bind_front(&BlotterPositionsView::on_flatten, this));
  commands->connect_flatten_all_signal(
    std::bind_front(&BlotterPositionsView::on_flatten_all, this));
  auto table = std::make_shared<ListToTableModel<PortfolioUpdateEntry>>(
    m_positions, static_cast<int>(PositionColumn::CURRENCY) + 1, &extract);
  m_table = TableViewBuilder(table).
    add_header_item(tr("Ticker"), TableHeaderItem::Order::ASCENDING).
    add_header_item(tr("Quantity"), tr("Qty")).
    add_header_item(tr("Side")).
    add_header_item(tr("Average Price"), tr("Avg Px")).
    add_header_item(tr("Profit/Loss"), tr("P/L")).
    add_header_item(tr("Cost Basis"), tr("CB")).
    add_header_item(tr("Currency"), tr("Curr")).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListMultiSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())).
    set_item_builder(&make_item).
    make();
  const auto DEFAULT_WIDTHS = std::array{90, 60, 50, 70, 90, 100, 70};
  for(auto i = 0; i != table->get_column_size(); ++i) {
    m_table->get_header().get_widths()->set(i, scale_width(DEFAULT_WIDTHS[i]));
    update_style(*m_table->get_header().get_item(i), [=] (auto& style) {
      style.get(Any() > TableHeaderItem::Label()).
        set(TextAlign(get_alignment(static_cast<PositionColumn>(i))));
    });
  }
  auto& body = m_table->get_body();
  body.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&body, &QWidget::customContextMenuRequested, this,
    &BlotterPositionsView::on_context_menu);
  auto& header = m_table->get_header();
  header.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&header, &QWidget::customContextMenuRequested, this,
    &BlotterPositionsView::on_header_menu);
  m_connections.add(m_positions->connect_operation_signal(
    [=, this] (const auto&) {
      on_update();
    }));
  m_connections.add(m_table->get_selection()->get_row_selection()->
    connect_operation_signal([=, this] (const auto&) {
      on_update();
    }));
  on_update();
  m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_vbox_layout(this);
  layout->addWidget(commands);
  layout->addWidget(m_table, 1);
  setFocusProxy(m_table);
}

const std::shared_ptr<PortfolioModel>&
    BlotterPositionsView::get_portfolio() const {
  return m_portfolio;
}

connection BlotterPositionsView::connect_flatten_signal(
    const FlattenSignal::slot_type& slot) const {
  return m_flatten_signal.connect(slot);
}

void BlotterPositionsView::keyPressEvent(QKeyEvent* event) {
  auto key = QKeySequence(event->modifiers() | event->key());
  if(auto operation = m_bindings->find_operation(key)) {
    event->accept();
    if(!event->isAutoRepeat()) {
      if(*operation == PositionKeyBindingsModel::Operation::FLATTEN) {
        on_flatten();
      } else {
        on_flatten_all();
      }
    }
    return;
  }
  QWidget::keyPressEvent(event);
}

void BlotterPositionsView::on_update() {
  if(m_positions->get_size() == 0) {
    m_can_flatten->set(FlattenState::NONE);
  } else if(m_table->get_selection()->get_row_selection()->get_size() == 0) {
    m_can_flatten->set(FlattenState::ALL);
  } else {
    m_can_flatten->set(FlattenState::SELECTED_AND_ALL);
  }
}

void BlotterPositionsView::on_flatten() {
  auto positions = std::vector<Position>();
  auto& rows = std::as_const(*m_table->get_selection()->get_row_selection());
  positions.reserve(rows.get_size());
  for(auto row : rows) {
    positions.push_back(m_positions->get(row).m_inventory.m_position);
  }
  if(!positions.empty()) {
    m_flatten_signal(positions);
  }
}

void BlotterPositionsView::on_flatten_all() {
  auto positions = std::vector<Position>();
  positions.reserve(m_positions->get_size());
  for(auto& entry : std::as_const(*m_positions)) {
    positions.push_back(entry.m_inventory.m_position);
  }
  if(!positions.empty()) {
    m_flatten_signal(positions);
  }
}

void BlotterPositionsView::on_context_menu(const QPoint& position) {
  auto& body = m_table->get_body();
  auto menu = new ContextMenu(body);
  auto label = tr("Flatten Position");
  auto& sequence = m_bindings->get_binding(
    PositionKeyBindingsModel::Operation::FLATTEN)->get();
  if(!sequence.isEmpty()) {
    label += tr(" (%1)").arg(sequence.toString(QKeySequence::NativeText));
  }
  if(m_can_flatten->get() == FlattenState::SELECTED_AND_ALL) {
    menu->add_action(
      label, std::bind_front(&BlotterPositionsView::on_flatten, this));
  } else {
    menu->add_disabled_action(label);
  }
  menu->add_separator();
  menu->add_action(tr("Export"), [=, this] {
    export_positions(*m_table);
  });
  menu->window()->setAttribute(Qt::WA_DeleteOnClose);
  menu->window()->move(body.mapToGlobal(position));
  menu->window()->show();
}

void BlotterPositionsView::on_header_menu(const QPoint& position) {
  auto& header = m_table->get_header();
  auto menu = new ContextMenu(header);
  auto columns = new ContextMenu(static_cast<QWidget&>(*menu));
  for(auto i = 1; i != header.get_items()->get_size(); ++i) {
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
