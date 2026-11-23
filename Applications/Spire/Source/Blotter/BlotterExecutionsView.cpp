#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <QFileDialog>
#include <QStandardPaths>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"
#include "Spire/Ui/OrderStatusFilterPanel.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class ExecutionColumn {
    TIME,
    ID,
    SIDE,
    TICKER,
    STATUS,
    QUANTITY,
    LAST_QUANTITY,
    PRICE,
    LAST_PRICE,
    MARKET,
    FLAG,
    EXECUTION_FEE,
    PROCESSING_FEE,
    MISCELLANEOUS_FEE,
    MESSAGE
  };

  Qt::Alignment get_alignment(ExecutionColumn column) {
    if(column == ExecutionColumn::QUANTITY ||
        column == ExecutionColumn::LAST_QUANTITY ||
        column == ExecutionColumn::PRICE ||
        column == ExecutionColumn::LAST_PRICE ||
        column == ExecutionColumn::EXECUTION_FEE ||
        column == ExecutionColumn::PROCESSING_FEE ||
        column == ExecutionColumn::MISCELLANEOUS_FEE) {
      return Qt::AlignRight | Qt::AlignVCenter;
    } else if(column == ExecutionColumn::MESSAGE) {
      return Qt::AlignLeft | Qt::AlignVCenter;
    }
    return Qt::AlignCenter;
  }

  AnyRef extract(const ExecutionReportEntry& entry, int index) {
    auto column = static_cast<ExecutionColumn>(index);
    auto& fields = entry.m_order->get_info().m_fields;
    auto& report = entry.m_report;
    if(column == ExecutionColumn::TIME) {
      return report.m_timestamp;
    } else if(column == ExecutionColumn::ID) {
      return report.m_id;
    } else if(column == ExecutionColumn::SIDE) {
      return fields.m_side;
    } else if(column == ExecutionColumn::TICKER) {
      return fields.m_ticker;
    } else if(column == ExecutionColumn::STATUS) {
      return report.m_status;
    } else if(column == ExecutionColumn::QUANTITY) {
      return fields.m_quantity;
    } else if(column == ExecutionColumn::LAST_QUANTITY) {
      return make_optional(report.m_last_quantity != 0, report.m_last_quantity);
    } else if(column == ExecutionColumn::PRICE) {
      return make_optional(fields.m_type == OrderType::LIMIT, fields.m_price);
    } else if(column == ExecutionColumn::LAST_PRICE) {
      return make_optional(report.m_last_quantity != 0, report.m_last_price);
    } else if(column == ExecutionColumn::MARKET) {
      if(report.m_last_quantity != 0) {
        return report.m_last_market;
      }
      return std::string();
    } else if(column == ExecutionColumn::FLAG) {
      return report.m_liquidity_flag;
    } else if(column == ExecutionColumn::EXECUTION_FEE) {
      return report.m_execution_fee;
    } else if(column == ExecutionColumn::PROCESSING_FEE) {
      return report.m_processing_fee;
    } else if(column == ExecutionColumn::MISCELLANEOUS_FEE) {
      return report.m_commission;
    }
    return report.m_text;
  }

  class ExecutionReportsFilter : public TableFilter {
    public:
      ExecutionReportsFilter(std::shared_ptr<OrderStatusListModel> statuses)
          : m_statuses(std::move(statuses)) {
        m_connection = m_statuses->connect_operation_signal(
          std::bind_front(&ExecutionReportsFilter::on_operation, this));
      }

      Filter get_filter(int column) override {
        if(column != static_cast<int>(ExecutionColumn::STATUS)) {
          return Filter::NONE;
        } else if(m_statuses->get_size() == 0) {
          return Filter::UNFILTERED;
        }
        return Filter::FILTERED;
      }

      QWidget* make_filter_widget(int column, QWidget& parent) override {
        if(column == static_cast<int>(ExecutionColumn::STATUS)) {
          return new OrderStatusFilterPanel(m_statuses, &parent);
        }
        return nullptr;
      }

      bool is_filtered(const TableModel& model, int row) const override {
        if(m_statuses->get_size() == 0) {
          return false;
        }
        auto status = model.get<OrderStatus>(
          row, static_cast<int>(ExecutionColumn::STATUS));
        auto& statuses = std::as_const(*m_statuses);
        return std::ranges::find(statuses, status) == statuses.end();
      }

      connection connect_filter_signal(
          const FilterSignal::slot_type& slot) const override {
        return m_filter_signal.connect(slot);
      }

    private:
      mutable FilterSignal m_filter_signal;
      std::shared_ptr<OrderStatusListModel> m_statuses;
      scoped_connection m_connection;

      void on_operation(const OrderStatusListModel::Operation& operation) {
        visit(operation, [&] <typename T> (const T&) requires(
            std::same_as<T, OrderStatusListModel::AddOperation> ||
            std::same_as<T, OrderStatusListModel::RemoveOperation> ||
            std::same_as<T, OrderStatusListModel::UpdateOperation>) {
          auto column = static_cast<int>(ExecutionColumn::STATUS);
          m_filter_signal(column, get_filter(column));
        });
      }
  };

  void export_reports(TableView& view) {
    auto file_name = QFileDialog::getSaveFileName(&view, QObject::tr("Save As"),
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      "/executions.csv", QObject::tr("CSV (*.csv)"));
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

  void initialize_menus(TableView& view) {
    auto& body = view.get_body();
    body.setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(&body, &QWidget::customContextMenuRequested, &view,
      [&] (const auto& position) {
        auto menu = new ContextMenu(view.get_body());
        menu->add_action(QObject::tr("Export"), [&] { export_reports(view); });
        menu->window()->setAttribute(Qt::WA_DeleteOnClose);
        menu->window()->move(view.get_body().mapToGlobal(position));
        menu->window()->show();
      });
    auto& header = view.get_header();
    header.setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(&header, &QWidget::customContextMenuRequested, &view,
      [&] (const auto& position) {
        auto& header = view.get_header();
        auto menu = new ContextMenu(header);
        auto columns = new ContextMenu(static_cast<QWidget&>(*menu));
        for(auto i = 0; i != header.get_items()->get_size(); ++i) {
          auto checked =
            columns->add_check_box(header.get_items()->get(i).m_name);
          checked->set(!header.get_item(i)->isHidden());
          checked->connect_update_signal([=, &view] (auto is_checked) {
            if(is_checked) {
              view.show_column(i);
            } else {
              view.hide_column(i);
            }
          });
        }
        menu->add_menu(QObject::tr("Table Columns"), *columns);
        menu->window()->setAttribute(Qt::WA_DeleteOnClose);
        menu->window()->move(header.mapToGlobal(position));
        menu->window()->show();
      });
  }
}

BlotterExecutionsView::BlotterExecutionsView(
    std::shared_ptr<ExecutionReportsModel> reports, QWidget* parent)
    : QWidget(parent),
      m_reports(std::move(reports)),
      m_statuses(std::make_shared<ArrayListModel<OrderStatus>>()) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table = std::make_shared<ListToTableModel<ExecutionReportEntry>>(
    m_reports, static_cast<int>(ExecutionColumn::MESSAGE) + 1, &extract);
  m_table = TableViewBuilder(table).
    add_header_item(tr("Time"), TableHeaderItem::Order::DESCENDING).
    add_header_item(tr("ID")).
    add_header_item(tr("Side")).
    add_header_item(tr("Ticker")).
    add_header_item(tr("Status"), TableFilter::Filter::UNFILTERED).
    add_header_item(tr("Quantity"), tr("Qty")).
    add_header_item(tr("Last Quantity"), tr("Last Qty")).
    add_header_item(tr("Price"), tr("Px")).
    add_header_item(tr("Last Price"), tr("Last Px")).
    add_header_item(tr("Market"), tr("Mkt")).
    add_header_item(tr("Flag")).
    add_header_item(tr("Execution Fee"), tr("Exec Fee")).
    add_header_item(tr("Processing Fee"), tr("Proc Fee")).
    add_header_item(tr("Miscellaneous Fee"), tr("Misc Fee")).
    add_header_item(tr("Message")).
    set_filter(std::make_shared<ExecutionReportsFilter>(m_statuses)).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListMultiSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())).
    set_item_builder([] (const auto& table, auto row, auto index) {
      auto item = TableView::default_item_builder(table, row, index);
      update_style(*item, [=] (auto& style) {
        auto alignment = get_alignment(static_cast<ExecutionColumn>(index));
        auto& rule = style.get(Any());
        rule.set(TextAlign(alignment));
        if(alignment.testFlag(Qt::AlignRight)) {
          rule.set(PaddingRight(scale_width(4)));
        } else if(alignment.testFlag(Qt::AlignLeft)) {
          rule.set(PaddingLeft(scale_width(4)));
        }
      });
      return item;
    }).
    make();
  const auto DEFAULT_WIDTHS = std::array{
    110, 100, 50, 90, 100, 60, 75, 70, 75, 70, 50, 75, 75, 75, 180};
  for(auto i = 0; i != table->get_column_size(); ++i) {
    m_table->get_header().get_widths()->set(i, scale_width(DEFAULT_WIDTHS[i]));
    update_style(*m_table->get_header().get_item(i), [=] (auto& style) {
      style.get(Any() > TableHeaderItem::Label()).
        set(TextAlign(get_alignment(static_cast<ExecutionColumn>(i))));
    });
  }
  initialize_menus(*m_table);
  m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *m_table);
  setFocusProxy(m_table);
}

const std::shared_ptr<ExecutionReportsModel>&
    BlotterExecutionsView::get_reports() const {
  return m_reports;
}
